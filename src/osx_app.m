//*****************************************************************
//
//	$file: osx_app.m $
//	$author: Martin Fouilleul $
//	$date: 16/05/2020 $
//	$revision: $
//	$note: (C) 2020 by Martin Fouilleul - all rights reserved $
//
//*****************************************************************

#include<stdlib.h> // malloc/free

#include"osx_app.h"
#include"lists.h"
#include"ringbuffer.h"
#include"memory.h"
#include"macro_helpers.h"
#include"platform_clock.h"

#define LOG_SUBSYSTEM "Application"

//--------------------------------------------------------------------
// mp window struct and utility functions
//--------------------------------------------------------------------

static mp_rect mp_osx_to_user_screen_rect(mp_rect rect)
{
	@autoreleasepool
	{
		NSRect screenRect = [[NSScreen mainScreen] frame];
		rect.y = screenRect.size.height - rect.y - rect.h;
	}
	return(rect);
}

static mp_rect mp_user_to_osx_screen_rect(mp_rect rect)
{
	@autoreleasepool
	{
		NSRect screenRect = [[NSScreen mainScreen] frame];
		rect.y = screenRect.size.height - rect.y - rect.h;
	}
	return(rect);
}

static void mp_window_update_rect_cache(mp_window_data* window)
{
	@autoreleasepool
	{
		NSRect frame = [window->nsWindow frame];
		window->frameRect = mp_osx_to_user_screen_rect((mp_rect){frame.origin.x, frame.origin.y, frame.size.width, frame.size.height});

		const NSRect contentRect = [[window->nsWindow contentView] frame];

		window->contentRect = (mp_rect){ contentRect.origin.x,
							 contentRect.origin.y,
							 contentRect.size.width,
							 contentRect.size.height };

		window->contentRect.y = window->frameRect.h - window->contentRect.y - window->contentRect.h;
	}
}

static u32 mp_osx_get_window_style_mask(mp_window_style style)
{
	u32 mask = 0;
	if(style & MP_WINDOW_STYLE_NO_TITLE)
	{
		mask = NSWindowStyleMaskBorderless;
	}
	else
	{
		mask = NSWindowStyleMaskTitled;
	}

	if(!(style & MP_WINDOW_STYLE_FIXED_SIZE))
	{
		mask |= NSWindowStyleMaskResizable;
	}
	if(!(style & MP_WINDOW_STYLE_NO_CLOSE))
	{
		mask |= NSWindowStyleMaskClosable;
	}
	if(!(style & MP_WINDOW_STYLE_NO_MINIFY))
	{
		mask |= NSWindowStyleMaskMiniaturizable;
	}
	return(mask);
}

//---------------------------------------------------------------
// App struct and utility functions
//---------------------------------------------------------------

const u32 MP_APP_MAX_WINDOWS = 128;
const u32 MP_APP_MAX_VIEWS = 128;

typedef struct mp_frame_stats
{
	f64 start;
	f64 workTime;
	f64 remainingTime;
	f64 targetFramePeriod;
} mp_frame_stats;

typedef struct mp_key_utf8
{
	u8 labelLen;
	char label[8];
} mp_key_utf8;

typedef struct mp_key_state
{
	u64 lastUpdate;
	u32 transitionCounter;
	bool down;
	bool clicked;
	bool doubleClicked;

} mp_key_state;

typedef struct mp_keyboard_state
{
	mp_key_state keys[MP_KEY_COUNT];
	mp_key_mods  mods;
} mp_keyboard_state;

typedef struct mp_mouse_state
{
	u64 lastUpdate;
	vec2 pos;
	vec2 delta;
	vec2 wheel;

	union
	{
		mp_key_state buttons[MP_MOUSE_BUTTON_COUNT];
		struct
		{
			mp_key_state left;
			mp_key_state right;
			mp_key_state middle;
			mp_key_state ext1;
			mp_key_state ext2;
		};
	};
} mp_mouse_state;

const u32 MP_INPUT_TEXT_BACKING_SIZE = 64;

typedef struct mp_text_state
{
	u64 lastUpdate;
	utf32 backing[MP_INPUT_TEXT_BACKING_SIZE];
	str32 codePoints;
} mp_text_state;

typedef struct mp_input_state
{
	u64 frameCounter;
	mp_keyboard_state keyboard;
	mp_mouse_state	mouse;
	mp_text_state text;
} mp_input_state;

typedef struct mp_app_data
{
	bool init;
	bool shouldQuit;

	str8 pendingPathDrop;
	mem_arena eventArena;
	mp_input_state inputState;
	ringbuffer eventQueue;

	mp_live_resize_callback liveResizeCallback;
	void* liveResizeData;

	//TODO: we should probably use a CVDisplayLink, but it complexifies things since
	//      it's called from another thread
	NSTimer* frameTimer;
	mp_event_callback eventCallback;
	void* eventData;

	mp_frame_stats frameStats;

	NSCursor* cursor;
	mp_window_data windowPool[MP_APP_MAX_WINDOWS];
	list_info windowFreeList;

	mp_view_data viewPool[MP_APP_MAX_VIEWS];
	list_info viewFreeList;

	TISInputSourceRef kbLayoutInputSource;
	void* kbLayoutUnicodeData;
	id kbLayoutListener;
	mp_key_utf8 mpKeyToLabel[256];

	int mpKeysToNative[MP_KEY_MAX];
	int nativeToMPKeys[256];

} mp_app_data;

static mp_app_data __mpAppData = {};

////////////////////////////////////////////////////////////////////////////////////////////////////////

void mp_init_window_handles()
{
	ListInit(&__mpAppData.windowFreeList);
	for(int i=0; i<MP_APP_MAX_WINDOWS; i++)
	{
		__mpAppData.windowPool[i].generation = 1;
		ListAppend(&__mpAppData.windowFreeList, &__mpAppData.windowPool[i].freeListElt);
	}
}

mp_window_data* mp_window_alloc()
{
	return(ListPopEntry(&__mpAppData.windowFreeList, mp_window_data, freeListElt));
}

mp_window_data* mp_window_ptr_from_handle(mp_window handle)
{
	u32 index = handle.h>>32;
	u32 generation = handle.h & 0xffffffff;
	if(index >= MP_APP_MAX_WINDOWS)
	{
		return(0);
	}
	mp_window_data* window = &__mpAppData.windowPool[index];
	if(window->generation != generation)
	{
		return(0);
	}
	else
	{
		return(window);
	}
}

mp_window mp_window_handle_from_ptr(mp_window_data* window)
{
	DEBUG_ASSERT(  (window - __mpAppData.windowPool) >= 0
	            && (window - __mpAppData.windowPool) < MP_APP_MAX_WINDOWS);

	u64 h = ((u64)(window - __mpAppData.windowPool))<<32
	      | ((u64)window->generation);

	return((mp_window){h});
}

void mp_window_recycle_ptr(mp_window_data* window)
{
	window->generation++;
	ListPush(&__mpAppData.windowFreeList, &window->freeListElt);
}


void mp_init_view_handles()
{
	ListInit(&__mpAppData.viewFreeList);
	for(int i=0; i<MP_APP_MAX_VIEWS; i++)
	{
		__mpAppData.viewPool[i].generation = 1;
		ListAppend(&__mpAppData.viewFreeList, &__mpAppData.viewPool[i].freeListElt);
	}
}

mp_view_data* mp_view_alloc()
{
	return(ListPopEntry(&__mpAppData.viewFreeList, mp_view_data, freeListElt));
}

mp_view_data* mp_view_ptr_from_handle(mp_view handle)
{
	u32 index = handle.h>>32;
	u32 generation = handle.h & 0xffffffff;
	if(index >= MP_APP_MAX_VIEWS)
	{
		return(0);
	}
	mp_view_data* view = &__mpAppData.viewPool[index];
	if(view->generation != generation)
	{
		return(0);
	}
	else
	{
		return(view);
	}
}

mp_view mp_view_handle_from_ptr(mp_view_data* view)
{
	DEBUG_ASSERT(  (view - __mpAppData.viewPool) >= 0
	            && (view - __mpAppData.viewPool) < MP_APP_MAX_VIEWS);

	u64 h = ((u64)(view - __mpAppData.viewPool))<<32
	      | ((u64)view->generation);

	return((mp_view){h});
}

void mp_view_recycle_ptr(mp_view_data* view)
{
	view->generation++;
	ListPush(&__mpAppData.viewFreeList, &view->freeListElt);
}

/*
void mp_app_set_process_event_callback(mp_app_process_event_callback callback, void* userData)
{
	DEBUG_ASSERT(callback);
	__mpAppData.userData = userData;
	__mpAppData.processEvent = callback;
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////


static void mp_init_osx_keys()
{
	memset(__mpAppData.nativeToMPKeys, MP_KEY_UNKNOWN, 256*sizeof(int));

	__mpAppData.nativeToMPKeys[0x1D] = MP_KEY_0;
	__mpAppData.nativeToMPKeys[0x12] = MP_KEY_1;
	__mpAppData.nativeToMPKeys[0x13] = MP_KEY_2;
	__mpAppData.nativeToMPKeys[0x14] = MP_KEY_3;
	__mpAppData.nativeToMPKeys[0x15] = MP_KEY_4;
	__mpAppData.nativeToMPKeys[0x17] = MP_KEY_5;
	__mpAppData.nativeToMPKeys[0x16] = MP_KEY_6;
	__mpAppData.nativeToMPKeys[0x1A] = MP_KEY_7;
	__mpAppData.nativeToMPKeys[0x1C] = MP_KEY_8;
	__mpAppData.nativeToMPKeys[0x19] = MP_KEY_9;
	__mpAppData.nativeToMPKeys[0x00] = MP_KEY_A;
	__mpAppData.nativeToMPKeys[0x0B] = MP_KEY_B;
	__mpAppData.nativeToMPKeys[0x08] = MP_KEY_C;
	__mpAppData.nativeToMPKeys[0x02] = MP_KEY_D;
	__mpAppData.nativeToMPKeys[0x0E] = MP_KEY_E;
	__mpAppData.nativeToMPKeys[0x03] = MP_KEY_F;
	__mpAppData.nativeToMPKeys[0x05] = MP_KEY_G;
	__mpAppData.nativeToMPKeys[0x04] = MP_KEY_H;
	__mpAppData.nativeToMPKeys[0x22] = MP_KEY_I;
	__mpAppData.nativeToMPKeys[0x26] = MP_KEY_J;
	__mpAppData.nativeToMPKeys[0x28] = MP_KEY_K;
	__mpAppData.nativeToMPKeys[0x25] = MP_KEY_L;
	__mpAppData.nativeToMPKeys[0x2E] = MP_KEY_M;
	__mpAppData.nativeToMPKeys[0x2D] = MP_KEY_N;
	__mpAppData.nativeToMPKeys[0x1F] = MP_KEY_O;
	__mpAppData.nativeToMPKeys[0x23] = MP_KEY_P;
	__mpAppData.nativeToMPKeys[0x0C] = MP_KEY_Q;
	__mpAppData.nativeToMPKeys[0x0F] = MP_KEY_R;
	__mpAppData.nativeToMPKeys[0x01] = MP_KEY_S;
	__mpAppData.nativeToMPKeys[0x11] = MP_KEY_T;
	__mpAppData.nativeToMPKeys[0x20] = MP_KEY_U;
	__mpAppData.nativeToMPKeys[0x09] = MP_KEY_V;
	__mpAppData.nativeToMPKeys[0x0D] = MP_KEY_W;
	__mpAppData.nativeToMPKeys[0x07] = MP_KEY_X;
	__mpAppData.nativeToMPKeys[0x10] = MP_KEY_Y;
	__mpAppData.nativeToMPKeys[0x06] = MP_KEY_Z;

	__mpAppData.nativeToMPKeys[0x27] = MP_KEY_APOSTROPHE;
	__mpAppData.nativeToMPKeys[0x2A] = MP_KEY_BACKSLASH;
	__mpAppData.nativeToMPKeys[0x2B] = MP_KEY_COMMA;
	__mpAppData.nativeToMPKeys[0x18] = MP_KEY_EQUAL;
	__mpAppData.nativeToMPKeys[0x32] = MP_KEY_GRAVE_ACCENT;
	__mpAppData.nativeToMPKeys[0x21] = MP_KEY_LEFT_BRACKET;
	__mpAppData.nativeToMPKeys[0x1B] = MP_KEY_MINUS;
	__mpAppData.nativeToMPKeys[0x2F] = MP_KEY_PERIOD;
	__mpAppData.nativeToMPKeys[0x1E] = MP_KEY_RIGHT_BRACKET;
	__mpAppData.nativeToMPKeys[0x29] = MP_KEY_SEMICOLON;
	__mpAppData.nativeToMPKeys[0x2C] = MP_KEY_SLASH;
	__mpAppData.nativeToMPKeys[0x0A] = MP_KEY_WORLD_1;

	__mpAppData.nativeToMPKeys[0x33] = MP_KEY_BACKSPACE;
	__mpAppData.nativeToMPKeys[0x39] = MP_KEY_CAPS_LOCK;
	__mpAppData.nativeToMPKeys[0x75] = MP_KEY_DELETE;
	__mpAppData.nativeToMPKeys[0x7D] = MP_KEY_DOWN;
	__mpAppData.nativeToMPKeys[0x77] = MP_KEY_END;
	__mpAppData.nativeToMPKeys[0x24] = MP_KEY_ENTER;
	__mpAppData.nativeToMPKeys[0x35] = MP_KEY_ESCAPE;
	__mpAppData.nativeToMPKeys[0x7A] = MP_KEY_F1;
	__mpAppData.nativeToMPKeys[0x78] = MP_KEY_F2;
	__mpAppData.nativeToMPKeys[0x63] = MP_KEY_F3;
	__mpAppData.nativeToMPKeys[0x76] = MP_KEY_F4;
	__mpAppData.nativeToMPKeys[0x60] = MP_KEY_F5;
	__mpAppData.nativeToMPKeys[0x61] = MP_KEY_F6;
	__mpAppData.nativeToMPKeys[0x62] = MP_KEY_F7;
	__mpAppData.nativeToMPKeys[0x64] = MP_KEY_F8;
	__mpAppData.nativeToMPKeys[0x65] = MP_KEY_F9;
	__mpAppData.nativeToMPKeys[0x6D] = MP_KEY_F10;
	__mpAppData.nativeToMPKeys[0x67] = MP_KEY_F11;
	__mpAppData.nativeToMPKeys[0x6F] = MP_KEY_F12;
	__mpAppData.nativeToMPKeys[0x69] = MP_KEY_F13;
	__mpAppData.nativeToMPKeys[0x6B] = MP_KEY_F14;
	__mpAppData.nativeToMPKeys[0x71] = MP_KEY_F15;
	__mpAppData.nativeToMPKeys[0x6A] = MP_KEY_F16;
	__mpAppData.nativeToMPKeys[0x40] = MP_KEY_F17;
	__mpAppData.nativeToMPKeys[0x4F] = MP_KEY_F18;
	__mpAppData.nativeToMPKeys[0x50] = MP_KEY_F19;
	__mpAppData.nativeToMPKeys[0x5A] = MP_KEY_F20;
	__mpAppData.nativeToMPKeys[0x73] = MP_KEY_HOME;
	__mpAppData.nativeToMPKeys[0x72] = MP_KEY_INSERT;
	__mpAppData.nativeToMPKeys[0x7B] = MP_KEY_LEFT;
	__mpAppData.nativeToMPKeys[0x3A] = MP_KEY_LEFT_ALT;
	__mpAppData.nativeToMPKeys[0x3B] = MP_KEY_LEFT_CONTROL;
	__mpAppData.nativeToMPKeys[0x38] = MP_KEY_LEFT_SHIFT;
	__mpAppData.nativeToMPKeys[0x37] = MP_KEY_LEFT_SUPER;
	__mpAppData.nativeToMPKeys[0x6E] = MP_KEY_MENU;
	__mpAppData.nativeToMPKeys[0x47] = MP_KEY_NUM_LOCK;
	__mpAppData.nativeToMPKeys[0x79] = MP_KEY_PAGE_DOWN;
	__mpAppData.nativeToMPKeys[0x74] = MP_KEY_PAGE_UP;
	__mpAppData.nativeToMPKeys[0x7C] = MP_KEY_RIGHT;
	__mpAppData.nativeToMPKeys[0x3D] = MP_KEY_RIGHT_ALT;
	__mpAppData.nativeToMPKeys[0x3E] = MP_KEY_RIGHT_CONTROL;
	__mpAppData.nativeToMPKeys[0x3C] = MP_KEY_RIGHT_SHIFT;
	__mpAppData.nativeToMPKeys[0x36] = MP_KEY_RIGHT_SUPER;
	__mpAppData.nativeToMPKeys[0x31] = MP_KEY_SPACE;
	__mpAppData.nativeToMPKeys[0x30] = MP_KEY_TAB;
	__mpAppData.nativeToMPKeys[0x7E] = MP_KEY_UP;

	__mpAppData.nativeToMPKeys[0x52] = MP_KEY_KP_0;
	__mpAppData.nativeToMPKeys[0x53] = MP_KEY_KP_1;
	__mpAppData.nativeToMPKeys[0x54] = MP_KEY_KP_2;
	__mpAppData.nativeToMPKeys[0x55] = MP_KEY_KP_3;
	__mpAppData.nativeToMPKeys[0x56] = MP_KEY_KP_4;
	__mpAppData.nativeToMPKeys[0x57] = MP_KEY_KP_5;
	__mpAppData.nativeToMPKeys[0x58] = MP_KEY_KP_6;
	__mpAppData.nativeToMPKeys[0x59] = MP_KEY_KP_7;
	__mpAppData.nativeToMPKeys[0x5B] = MP_KEY_KP_8;
	__mpAppData.nativeToMPKeys[0x5C] = MP_KEY_KP_9;
	__mpAppData.nativeToMPKeys[0x45] = MP_KEY_KP_ADD;
	__mpAppData.nativeToMPKeys[0x41] = MP_KEY_KP_DECIMAL;
	__mpAppData.nativeToMPKeys[0x4B] = MP_KEY_KP_DIVIDE;
	__mpAppData.nativeToMPKeys[0x4C] = MP_KEY_KP_ENTER;
	__mpAppData.nativeToMPKeys[0x51] = MP_KEY_KP_EQUAL;
	__mpAppData.nativeToMPKeys[0x43] = MP_KEY_KP_MULTIPLY;
	__mpAppData.nativeToMPKeys[0x4E] = MP_KEY_KP_SUBTRACT;

	memset(__mpAppData.mpKeysToNative, 0, sizeof(int)*MP_KEY_COUNT);
	for(int nativeKey=0; nativeKey<256; nativeKey++)
	{
		mp_key_code mpKey = __mpAppData.nativeToMPKeys[nativeKey];
		if(mpKey)
		{
			__mpAppData.mpKeysToNative[mpKey] = nativeKey;
		}
	}
}

static int mp_convert_osx_key(unsigned short nsCode)
{
	if(nsCode >= 265)
	{
		return(MP_KEY_UNKNOWN);
	}
	else
	{
		return(__mpAppData.nativeToMPKeys[nsCode]);
	}
}

static mp_key_mods mp_convert_osx_mods(NSUInteger nsFlags)
{
	mp_key_mods mods = MP_KEYMOD_NONE;
	if(nsFlags & NSEventModifierFlagShift)
	{
		mods |= MP_KEYMOD_SHIFT;
	}
	if(nsFlags & NSEventModifierFlagControl)
	{
		mods |= MP_KEYMOD_CTRL;
	}
	if(nsFlags & NSEventModifierFlagOption)
	{
		mods |= MP_KEYMOD_ALT;
	}
	if(nsFlags & NSEventModifierFlagCommand)
	{
		mods |= MP_KEYMOD_CMD;
	}
	return(mods);
}

static void mp_update_keyboard_layout()
{
	if(__mpAppData.kbLayoutInputSource)
	{
		CFRelease(__mpAppData.kbLayoutInputSource);
		__mpAppData.kbLayoutInputSource = 0;
		__mpAppData.kbLayoutUnicodeData = nil;
	}

	__mpAppData.kbLayoutInputSource = TISCopyCurrentKeyboardLayoutInputSource();
	if(!__mpAppData.kbLayoutInputSource)
	{
		LOG_ERROR("Failed to load keyboard layout input source");
	}

	__mpAppData.kbLayoutUnicodeData = TISGetInputSourceProperty(__mpAppData.kbLayoutInputSource,
	                                                            kTISPropertyUnicodeKeyLayoutData);
	if(!__mpAppData.kbLayoutUnicodeData)
	{
		LOG_ERROR("Failed to load keyboard layout unicode data");
	}

	memset(__mpAppData.mpKeyToLabel, 0, sizeof(mp_key_utf8)*MP_KEY_COUNT);

	for(int key=0; key<MP_KEY_COUNT; key++)
	{
		//TODO: check that the key is printable
		int nativeKey = __mpAppData.mpKeysToNative[key];

		UInt32 deadKeyState = 0;
		UniChar characters[4];
		UniCharCount characterCount = 0;

		if(UCKeyTranslate((UCKeyboardLayout*)[(NSData*) __mpAppData.kbLayoutUnicodeData bytes],
	             		nativeKey,
	             		kUCKeyActionDisplay,
	             		0,
	             		LMGetKbdType(),
	             		kUCKeyTranslateNoDeadKeysBit,
	             		&deadKeyState,
	             		sizeof(characters) / sizeof(UniChar),
	             		&characterCount,
	             		characters) != noErr)
		{
			__mpAppData.mpKeyToLabel[key].labelLen = 0;
		}
		else
		{
			NSString* nsString = [[NSString alloc] initWithCharacters: characters length: characterCount];
			const char* cstring = [nsString UTF8String];
			u32 len = strlen(cstring);
			__mpAppData.mpKeyToLabel[key].labelLen = minimum(len, 8);
			memcpy(__mpAppData.mpKeyToLabel[key].label, cstring, __mpAppData.mpKeyToLabel[key].labelLen);
		}
	}
}

str8 mp_key_to_label(mp_key_code key)
{
	mp_key_utf8* keyInfo = &(__mpAppData.mpKeyToLabel[key]);
	str8 label = str8_from_buffer(keyInfo->labelLen, keyInfo->label);
	return(label);
}

mp_key_code mp_label_to_key(str8 label)
{
	mp_key_code res = MP_KEY_UNKNOWN;
	for(int key=0; key<MP_KEY_COUNT; key++)
	{
		str8 keyLabel = mp_key_to_label(key);
		if(  keyLabel.len == label.len
		  && !strncmp(keyLabel.ptr, label.ptr, label.len))
		{
			res = key;
			break;
		}
	}
	return(res);
}

@interface MPKeyboardLayoutListener : NSObject
@end

@implementation MPKeyboardLayoutListener

- (void)selectedKeyboardInputSourceChanged:(NSObject* )object
{
	mp_update_keyboard_layout();
}

@end

void mp_install_keyboard_layout_listener()
{
	__mpAppData.kbLayoutListener = [[MPKeyboardLayoutListener alloc] init];
	[[NSDistributedNotificationCenter defaultCenter]
		addObserver: __mpAppData.kbLayoutListener
		selector: @selector(selectedKeyboardInputSourceChanged:)
		name:(__bridge NSString*)kTISNotifySelectedKeyboardInputSourceChanged
		object:nil];
}

static void mp_update_key_mods(mp_key_mods mods)
{
	__mpAppData.inputState.keyboard.mods = mods;
}

static void mp_update_key_state(mp_key_state* key, bool down)
{
	u64 frameCounter = __mpAppData.inputState.frameCounter;
	if(key->lastUpdate != frameCounter)
	{
		key->transitionCounter = 0;
		key->clicked = false;
		key->doubleClicked = false;
		key->lastUpdate = frameCounter;
	}

	key->transitionCounter++;
	key->down = down;
}

static void mp_update_mouse_move(f32 x, f32 y, f32 deltaX, f32 deltaY)
{
	u64 frameCounter = __mpAppData.inputState.frameCounter;
	mp_mouse_state* mouse = &__mpAppData.inputState.mouse;
	if(mouse->lastUpdate != frameCounter)
	{
		mouse->delta = (vec2){0, 0};
		mouse->wheel = (vec2){0, 0};
		mouse->lastUpdate = frameCounter;
	}
	mouse->pos = (vec2){x, y};
	mouse->delta.x += deltaX;
	mouse->delta.y += deltaY;
}

static void mp_update_mouse_wheel(f32 deltaX, f32 deltaY)
{
	u64 frameCounter = __mpAppData.inputState.frameCounter;
	mp_mouse_state* mouse = &__mpAppData.inputState.mouse;
	if(mouse->lastUpdate != frameCounter)
	{
		mouse->delta = (vec2){0, 0};
		mouse->wheel = (vec2){0, 0};
		mouse->lastUpdate = frameCounter;
	}
	mouse->wheel.x += deltaX;
	mouse->wheel.y += deltaY;
}

static void mp_update_text(utf32 codepoint)
{
	u64 frameCounter = __mpAppData.inputState.frameCounter;
	mp_text_state* text = &__mpAppData.inputState.text;

	if(text->lastUpdate != frameCounter)
	{
		text->codePoints.len = 0;
		text->lastUpdate = frameCounter;
	}

	text->codePoints.ptr = text->backing;
	if(text->codePoints.len < MP_INPUT_TEXT_BACKING_SIZE)
	{
		text->codePoints.ptr[text->codePoints.len] = codepoint;
		text->codePoints.len++;
	}
	else
	{
		LOG_WARNING("too many input codepoints per frame, dropping input");
	}
}

static void mp_queue_event(mp_event* event)
{
	if(ringbuffer_write_available(&__mpAppData.eventQueue) < sizeof(mp_event))
	{
		LOG_ERROR("event queue full\n");
	}
	else
	{
		u32 written = ringbuffer_write(&__mpAppData.eventQueue, sizeof(mp_event), (u8*)event);
		DEBUG_ASSERT(written == sizeof(mp_event));
	}
}

static void mp_dispatch_event(mp_event* event)
{
	if(__mpAppData.eventCallback)
	{
		__mpAppData.eventCallback(*event, __mpAppData.eventData);
	}
	else
	{
		mp_queue_event(event);
	}
}

//---------------------------------------------------------------
// Application and app delegate
//---------------------------------------------------------------

@interface MPApplication : NSApplication
@end

@implementation MPApplication
-(void)noOpThread:(id)object
{}
@end

@interface MPAppDelegate : NSObject <NSApplicationDelegate>
-(id)init;
@end

@implementation MPAppDelegate

-(id)init
{
	self = [super init];
	[[NSAppleEventManager sharedAppleEventManager] setEventHandler:self
	                                               andSelector:@selector(handleAppleEvent:withReplyEvent:)
	                                               forEventClass:kInternetEventClass
	                                               andEventID:kAEGetURL];

	return(self);
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	//NOTE: We set shouldQuit to true and send a Quit event
	//	We then return a value to cancel the direct termination because we still
	//	want to execte the code after mp_event_loop(). If the user didn't set shouldQuit to
	//	false, mp_event_loop() will exit, and the user can execute any cleanup needed and
	//	exit the program.

	__mpAppData.shouldQuit = true;
	mp_event event = {};
	event.type = MP_EVENT_QUIT;
	mp_dispatch_event(&event);

	return(NSTerminateCancel);
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification
{@autoreleasepool{

	//NOTE(martin): add a menu for quit, and a corresponding key equivalent.
	//		this allows to quit the application when there is no window
	//		left to catch our Cmd-Q key equivalent
	NSMenu* bar = [[NSMenu alloc] init];
	[NSApp setMainMenu:bar];

	NSMenuItem* appMenuItem =
	[bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
	NSMenu* appMenu = [[NSMenu alloc] init];
	[appMenuItem setSubmenu:appMenu];

	[appMenu addItemWithTitle: @"Quit"
		 action: @selector(terminate:)
		 keyEquivalent: @"q"];

}}

- (void)timerElapsed:(NSTimer*)timer
{
	mp_event event = {};
	event.type = MP_EVENT_FRAME;
	mp_dispatch_event(&event);
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{@autoreleasepool{
	//WARN(martin): the order of these calls seem to matter a lot for properly showing the menu bar
	//              with other orderings, the menu doesn't display before the application is put out of
	//              focus and on focus again... This is flaky undocumented behaviour, so although it is
	//		fixed by the current ordering, we expect the problem to show up again in future
	//		versions of macOS.

	//NOTE(martin): send a dummy event to wake-up the run loop and exit from the run loop.

	NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
					    location:NSMakePoint(0, 0)
				       modifierFlags:0
					   timestamp:0
					windowNumber:0
					     context:nil
					     subtype:0
					       data1:0
					       data2:0];

	[NSApp postEvent:event atStart:YES];
	[NSApp stop:nil];
}}

- (BOOL)application:(NSApplication *)application openFile:(NSString *)filename
{
	mp_event event = {};
	event.window = (mp_window){0};
	event.type = MP_EVENT_PATHDROP;
	event.path = str8_push_cstring(&__mpAppData.eventArena, [filename UTF8String]);

	mp_dispatch_event(&event);
	return(YES);
}

- (void)handleAppleEvent:(NSAppleEventDescriptor*)appleEvent withReplyEvent:(NSAppleEventDescriptor*)replyEvent
{
	NSString* nsPath = [[appleEvent paramDescriptorForKeyword:keyDirectObject] stringValue];

	mp_event event = {};
	event.window = (mp_window){0};
	event.type = MP_EVENT_PATHDROP;
	event.path = str8_push_cstring(&__mpAppData.eventArena, [nsPath UTF8String]);

	mp_dispatch_event(&event);
}

@end // @implementation MPAppDelegate

//---------------------------------------------------------------
// Custom NSWindow
//---------------------------------------------------------------

@implementation MPNativeWindow
- (id)initWithMPWindow:(mp_window_data*) window contentRect:(NSRect) rect styleMask:(uint32) style
{
	mpWindow = window;
	return([self initWithContentRect:rect styleMask:style backing:NSBackingStoreBuffered defer:NO]);
}
- (BOOL)canBecomeKeyWindow
{
	return(!(mpWindow->style & MP_WINDOW_STYLE_NO_FOCUS));
}
/*
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
	if([sender draggingSourceOperationMask] & NSDragOperationGeneric)
	{
		return NSDragOperationGeneric;
	}
	return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{@autoreleasepool
{
	NSPasteboard *pasteboard = [sender draggingPasteboard];
	NSArray *types = [NSArray arrayWithObject:NSFilenamesPboardType];
	NSString *desiredType = [pasteboard availableTypeFromArray:types];

	NSData *data;
	NSArray *array;
	NSPoint point;

	if (desiredType == nil) {
    	return NO;
	}

	data = [pasteboard dataForType:desiredType];
	if (data == nil) {
    	return NO;
	}

	SDL_assert([desiredType isEqualToString:NSFilenamesPboardType]);
	array = [pasteboard propertyListForType:@"NSFilenamesPboardType"];

	// Code addon to update the mouse location
	point = [sender draggingLocation];
	mouse = SDL_GetMouse();
	x = (int)point.x;
	y = (int)(sdlwindow->h - point.y);
	if (x >= 0 && x < sdlwindow->w && y >= 0 && y < sdlwindow->h) {
    	SDL_SendMouseMotion(sdlwindow, mouse->mouseID, 0, x, y);
	}
	// Code addon to update the mouse location

	for (NSString *path in array) {
    	NSURL *fileURL = [NSURL fileURLWithPath:path];
    	NSNumber *isAlias = nil;

    	[fileURL getResourceValue:&isAlias forKey:NSURLIsAliasFileKey error:nil];

    	// If the URL is an alias, resolve it.
    	if ([isAlias boolValue]) {
        	NSURLBookmarkResolutionOptions opts = NSURLBookmarkResolutionWithoutMounting | NSURLBookmarkResolutionWithoutUI;
        	NSData *bookmark = [NSURL bookmarkDataWithContentsOfURL:fileURL error:nil];
        	if (bookmark != nil) {
            	NSURL *resolvedURL = [NSURL URLByResolvingBookmarkData:bookmark
                                                           	options:opts
                                                     	relativeToURL:nil
                                               	bookmarkDataIsStale:nil
                                                             	error:nil];

            	if (resolvedURL != nil) {
                	fileURL = resolvedURL;
            	}
        	}
    	}

    	if (!SDL_SendDropFile(sdlwindow, [[fileURL path] UTF8String])) {
        	return NO;
    	}
	}

	SDL_SendDropComplete(sdlwindow);
	return YES;
}

- (BOOL)wantsPeriodicDraggingUpdates;
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;
*/

@end //@implementation MPNativeWindow

//---------------------------------------------------------------
// Custom NSWindow delegate
//---------------------------------------------------------------

@interface MPNativeWindowDelegate : NSObject
{
	mp_window_data* mpWindow;
}
- (id)initWithMPWindow:(mp_window_data*) window;
@end

@implementation MPNativeWindowDelegate

- (id)initWithMPWindow:(mp_window_data*) window
{
	self = [super init];
	if(self != nil)
	{
		mpWindow = window;
	}
	return(self);
}

- (void)windowDidBecomeKey:(NSNotification*)notification
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(mpWindow);
	event.type = MP_EVENT_WINDOW_FOCUS;

	mpWindow->hidden = false;

	mp_dispatch_event(&event);
}

- (void)windowDidResignKey:(NSNotification*)notification
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(mpWindow);
	event.type = MP_EVENT_WINDOW_UNFOCUS;

	mp_dispatch_event(&event);
}

- (void)windowDidMove:(NSNotification *)notification
{
	const NSRect contentRect = [[mpWindow->nsWindow contentView] frame];

	mp_window_update_rect_cache(mpWindow);

	mp_event event = {};
	event.window = mp_window_handle_from_ptr(mpWindow);
	event.type = MP_EVENT_WINDOW_MOVE;
	event.frame.rect.x = contentRect.origin.x;
	event.frame.rect.y = contentRect.origin.y;
	event.frame.rect.w = contentRect.size.width;
	event.frame.rect.h = contentRect.size.height;

	mp_dispatch_event(&event);
}

- (void)windowDidResize:(NSNotification *)notification
{
	const NSRect contentRect = [[mpWindow->nsWindow contentView] frame];

	mp_window_update_rect_cache(mpWindow);

	mp_event event = {};
	event.window = mp_window_handle_from_ptr(mpWindow);
	event.type = MP_EVENT_WINDOW_RESIZE;
	event.frame.rect.x = contentRect.origin.x;
	event.frame.rect.y = contentRect.origin.y;
	event.frame.rect.w = contentRect.size.width;
	event.frame.rect.h = contentRect.size.height;

	mp_rect viewFrame = {0, 0, contentRect.size.width, contentRect.size.height};
	mp_view_set_frame(mpWindow->mainView, viewFrame);


	if(__mpAppData.liveResizeCallback)
	{
		__mpAppData.liveResizeCallback(event, __mpAppData.liveResizeData);
	}

	//TODO: also ensure we don't overflow the queue during live resize...
	mp_dispatch_event(&event);
}

-(void)windowWillStartLiveResize:(NSNotification *)notification
{
	//TODO
}

-(void)windowDidEndLiveResize:(NSNotification *)notification
{
	//TODO
}

- (void)windowWillClose:(NSNotification *)notification
{
	mpWindow->nsWindow = nil;
	[mpWindow->nsView release];
	mpWindow->nsView = nil;
	[mpWindow->nsWindowDelegate release];
	mpWindow->nsWindowDelegate = nil;

	mp_window_recycle_ptr(mpWindow);
}

- (BOOL)windowShouldClose:(id)sender
{
	mpWindow->shouldClose = true;

	mp_event event = {};
	event.window = mp_window_handle_from_ptr(mpWindow);
	event.type = MP_EVENT_WINDOW_CLOSE;

	mp_dispatch_event(&event);

	return(mpWindow->shouldClose);
}

@end //@implementation MPNativeWindowDelegate

//---------------------------------------------------------------
// Custom NSView
//---------------------------------------------------------------

@interface MPNativeView : NSView <NSTextInputClient>
{
	mp_window_data* window;
	NSTrackingArea* trackingArea;
	NSMutableAttributedString* markedText;
}
- (id)initWithMPWindow:(mp_window_data*) mpWindow;
@end

@implementation MPNativeView

- (id)initWithMPWindow:(mp_window_data*) mpWindow
{
	self = [super init];
	if(self != nil)
	{
		window = mpWindow;
		mpWindow->nsView = self;

		NSTrackingAreaOptions trackingOptions =	  NSTrackingMouseEnteredAndExited
							| NSTrackingMouseMoved
							| NSTrackingCursorUpdate
							| NSTrackingActiveInActiveApp	   //TODO maybe change that to allow multi-window mouse events...
							| NSTrackingEnabledDuringMouseDrag
							| NSTrackingInVisibleRect
							| NSTrackingAssumeInside ;

		trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:trackingOptions owner:self userInfo:nil];
		[self addTrackingArea:trackingArea];
		markedText = [[NSMutableAttributedString alloc] init];
	}
	return(self);
}

- (void)dealloc
{
	[trackingArea release];
	[markedText release];
	[super dealloc];
}

-(void)drawRect:(NSRect)dirtyRect
{
	if(window->style & MP_WINDOW_STYLE_NO_TITLE)
	{
		[NSGraphicsContext saveGraphicsState];
		NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:[self frame] xRadius:5 yRadius:5];
		[path addClip];
		[[NSColor whiteColor] set];
		NSRectFill([self frame]);
	}

	if(window->style & MP_WINDOW_STYLE_NO_TITLE)
	{
		[NSGraphicsContext restoreGraphicsState];
		[window->nsWindow invalidateShadow];
	}
}

- (BOOL)acceptsFirstReponder
{
	return(YES);
}

- (void)cursorUpdate:(NSEvent*)event
{
	if(__mpAppData.cursor)
	{
		[__mpAppData.cursor set];
	}
	else
	{
		[[NSCursor arrowCursor] set];
	}
}

- (void)mouseDown:(NSEvent *)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_BUTTON;
	event.key.action = MP_KEY_PRESS;
	event.key.code = MP_MOUSE_LEFT;
	event.key.mods = mp_convert_osx_mods([nsEvent modifierFlags]);
	event.key.clickCount = [nsEvent clickCount];

	mp_update_key_state(&__mpAppData.inputState.mouse.buttons[event.key.code], true);
	if(event.key.clickCount >= 1)
	{
		__mpAppData.inputState.mouse.buttons[event.key.code].clicked = true;
	}
	if(event.key.clickCount >= 2)
	{

		__mpAppData.inputState.mouse.buttons[event.key.code].doubleClicked = true;
	}

	mp_dispatch_event(&event);

	[window->nsWindow makeFirstResponder:self];
}

- (void)mouseUp:(NSEvent*)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_BUTTON;
	event.key.action = MP_KEY_RELEASE;
	event.key.code = MP_MOUSE_LEFT;
	event.key.mods = mp_convert_osx_mods([nsEvent modifierFlags]);
	event.key.clickCount = [nsEvent clickCount];

	mp_update_key_state(&__mpAppData.inputState.mouse.buttons[event.key.code], false);

	mp_dispatch_event(&event);
}

- (void)rightMouseDown:(NSEvent*)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_BUTTON;
	event.key.action = MP_KEY_PRESS;
	event.key.code = MP_MOUSE_RIGHT;
	event.key.mods = mp_convert_osx_mods([nsEvent modifierFlags]);

	mp_update_key_state(&__mpAppData.inputState.mouse.buttons[event.key.code], true);

	mp_dispatch_event(&event);
}

- (void)rightMouseUp:(NSEvent*)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_BUTTON;
	event.key.action = MP_KEY_RELEASE;
	event.key.code = MP_MOUSE_RIGHT;
	event.key.mods = mp_convert_osx_mods([nsEvent modifierFlags]);

		mp_update_key_state(&__mpAppData.inputState.mouse.buttons[event.key.code], false);

	mp_dispatch_event(&event);
}

- (void)otherMouseDown:(NSEvent*)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_BUTTON;
	event.key.action = MP_KEY_PRESS;
	event.key.code = [nsEvent buttonNumber];
	event.key.mods = mp_convert_osx_mods([nsEvent modifierFlags]);

	mp_update_key_state(&__mpAppData.inputState.mouse.buttons[event.key.code], true);

	mp_dispatch_event(&event);
}

- (void)otherMouseUp:(NSEvent*)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_BUTTON;
	event.key.action = MP_KEY_RELEASE;
	event.key.code = [nsEvent buttonNumber];
	event.key.mods = mp_convert_osx_mods([nsEvent modifierFlags]);

	mp_update_key_state(&__mpAppData.inputState.mouse.buttons[event.key.code], false);

	mp_dispatch_event(&event);
}

- (void)mouseDragged:(NSEvent*)nsEvent
{
	[self mouseMoved:nsEvent];
}

- (void)mouseMoved:(NSEvent*)nsEvent
{
	NSPoint p = [self convertPoint:[nsEvent locationInWindow] fromView:nil];

	NSRect frame = [[window->nsWindow contentView] frame];
	mp_event event = {};
	event.type = MP_EVENT_MOUSE_MOVE;
	event.window = mp_window_handle_from_ptr(window);
	event.move.x = p.x;
	event.move.y = p.y;
	event.move.deltaX = [nsEvent deltaX];
	event.move.deltaY = -[nsEvent deltaY];
	event.move.mods = mp_convert_osx_mods([nsEvent modifierFlags]);

	mp_update_mouse_move(p.x, p.y, event.move.deltaX, event.move.deltaY);

	mp_dispatch_event(&event);
}

- (void)scrollWheel:(NSEvent*)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_WHEEL;

	double factor = [nsEvent hasPreciseScrollingDeltas] ? 0.1 : 1.0;
	event.move.x = 0;
	event.move.y = 0;
	event.move.deltaX = [nsEvent scrollingDeltaX]*factor;
	event.move.deltaY = [nsEvent scrollingDeltaY]*factor;
	event.move.mods = mp_convert_osx_mods([nsEvent modifierFlags]);

	mp_update_mouse_wheel(event.move.deltaX, event.move.deltaY);

	mp_dispatch_event(&event);
}

- (void)mouseExited:(NSEvent *)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_LEAVE;
	mp_dispatch_event(&event);
}

- (void)mouseEntered:(NSEvent *)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_ENTER;
	mp_dispatch_event(&event);
}



- (void)keyDown:(NSEvent*)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_KEYBOARD_KEY;
	event.key.action = MP_KEY_PRESS;
	event.key.code = mp_convert_osx_key([nsEvent keyCode]);
	event.key.mods = mp_convert_osx_mods([nsEvent modifierFlags]);

	str8 label = mp_key_to_label(event.key.code);
	event.key.labelLen = label.len;
	memcpy(event.key.label, label.ptr, label.len);

	mp_update_key_state(&__mpAppData.inputState.keyboard.keys[event.key.code], true);

	mp_dispatch_event(&event);

	[self interpretKeyEvents:@[nsEvent]];
}

- (void)keyUp:(NSEvent*)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_KEYBOARD_KEY;
	event.key.action = MP_KEY_RELEASE;
	event.key.code = mp_convert_osx_key([nsEvent keyCode]);
	event.key.mods = mp_convert_osx_mods([nsEvent modifierFlags]);

	mp_update_key_state(&__mpAppData.inputState.keyboard.keys[event.key.code], false);

	mp_dispatch_event(&event);
}

- (void) flagsChanged:(NSEvent*)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_KEYBOARD_MODS;
	event.key.mods = mp_convert_osx_mods([nsEvent modifierFlags]);

	mp_update_key_mods(event.key.mods);

	mp_dispatch_event(&event);
}

- (BOOL)performKeyEquivalent:(NSEvent*)nsEvent
{
	if([nsEvent modifierFlags] & NSEventModifierFlagCommand)
	{
		if([nsEvent charactersIgnoringModifiers] == [NSString stringWithUTF8String:"w"])
		{
			[window->nsWindow performClose:self];
			return(YES);
		}
		else if([nsEvent charactersIgnoringModifiers] == [NSString stringWithUTF8String:"q"])
		{
			__mpAppData.shouldQuit = true;

			mp_event event = {};
			event.type = MP_EVENT_QUIT;

			mp_dispatch_event(&event);

			//[NSApp terminate:self];
			return(YES);
		}
	}

	return([super performKeyEquivalent:nsEvent]);
}

- (BOOL)hasMarkedText
{
	return([markedText length] > 0);
}

static const NSRange kEmptyRange = { NSNotFound, 0 };

- (NSRange)markedRange
{
	if([markedText length] > 0)
	{
		return(NSMakeRange(0, [markedText length] - 1));
	}
	else
	{
		return(kEmptyRange);
	}
}

- (NSRange)selectedRange
{
	return(kEmptyRange);
}

- (void)setMarkedText:(id)string
        selectedRange:(NSRange)selectedRange
     replacementRange:(NSRange)replacementRange
{
	[markedText release];

	if([string isKindOfClass:[NSAttributedString class]])
	{
		markedText = [[NSMutableAttributedString alloc] initWithAttributedString:string];
	}
	else
	{
		markedText = [[NSMutableAttributedString alloc] initWithString:string];
	}
}

- (void)unmarkText
{
	[[markedText mutableString] setString:@""];
}

- (NSArray*)validAttributesForMarkedText
{
	return([NSArray array]);
}

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range
                                               actualRange:(NSRangePointer)actualRange
{
	return(nil);
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
	return(0);
}

- (NSRect)firstRectForCharacterRange:(NSRange)range
                         actualRange:(NSRangePointer)actualRange
{
	NSRect frame = [window->nsView frame];
	return(NSMakeRect(frame.origin.x, frame.origin.y, 0.0, 0.0));
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
	NSString* characters;
	NSEvent* nsEvent = [NSApp currentEvent];
	mp_key_mods mods = mp_convert_osx_mods([nsEvent modifierFlags]);

	if([string isKindOfClass:[NSAttributedString class]])
	{
		characters = [string string];
	}
	else
	{
		characters = (NSString*) string;
	}

	NSRange range = NSMakeRange(0, [characters length]);
	while (range.length)
	{
		utf32 codepoint = 0;

		if ([characters getBytes:&codepoint
				maxLength:sizeof(codepoint)
				usedLength:NULL
				encoding:NSUTF32StringEncoding
				options:0
				range:range
				remainingRange:&range])
		{
			if(codepoint >= 0xf700 && codepoint <= 0xf7ff)
			{
				continue;
			}

			mp_event event = {};
			event.window = mp_window_handle_from_ptr(window);
			event.type = MP_EVENT_KEYBOARD_CHAR;
			event.character.codepoint = codepoint;

			str8 seq = utf8_encode(event.character.sequence, event.character.codepoint);
			event.character.seqLen = seq.len;

			mp_update_text(codepoint);

			mp_dispatch_event(&event);
		}
	}
	[self unmarkText];
}

- (void)doCommandBySelector:(SEL)selector
{
}


@end //@implementation MPNativeView

/*
void mp_sleep_nanoseconds(u64 nanoseconds)
{
	timespec rqtp;
	rqtp.tv_sec = nanoseconds / 1000000000;
	rqtp.tv_nsec = nanoseconds - rqtp.tv_sec * 1000000000;
	nanosleep(&rqtp, 0);
}

static mach_timebase_info_data_t __machTimeBase__ = {1,1};

u64 mp_get_elapsed_nanoseconds()
{
	//NOTE(martin): according to the documentation, mach_absolute_time() does not
	//              increment when the system is asleep
	u64 now = mach_absolute_time();
	now *= __machTimeBase__.numer;
	now /= __machTimeBase__.denom;
	return(now);
}

f64 mp_get_elapsed_seconds()
{
	return(1.e-9*(f64)mp_get_elapsed_nanoseconds());
}
*/
/*
CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink,
                             const CVTimeStamp *inNow,
                             const CVTimeStamp *inOutputTime,
                             CVOptionFlags flagsIn,
                             CVOptionFlags *flagsOut,
                             void *displayLinkContext)
{
	if(__mpAppData.displayRefreshCallback)
	{
		__mpAppData.displayRefreshCallback(__mpAppData.displayRefreshData);
	}

	return(0);
}
*/

//***************************************************************
//			public API
//***************************************************************

//---------------------------------------------------------------
// App public API
//---------------------------------------------------------------

void mp_init()
{@autoreleasepool {
	if(!__mpAppData.init)
	{
		memset(&__mpAppData, 0, sizeof(__mpAppData));

		mem_arena_init(&__mpAppData.eventArena);

		mp_clock_init();

		LOG_MESSAGE("init keys\n");
		mp_init_osx_keys();
		mp_update_keyboard_layout();
		mp_install_keyboard_layout_listener();

		LOG_MESSAGE("init handles\n");
		mp_init_window_handles();
		mp_init_view_handles();

		LOG_MESSAGE("init event queue\n");
		ringbuffer_init(&__mpAppData.eventQueue, 16);

		[MPApplication sharedApplication];
		MPAppDelegate* delegate = [[MPAppDelegate alloc] init];
		[NSApp setDelegate: delegate];

		[NSThread detachNewThreadSelector:@selector(noOpThread:)
		                         toTarget:NSApp
		                         withObject:nil];

		__mpAppData.init = true;

		LOG_MESSAGE("run application\n");
		[NSApp run];

		/*
		CGDirectDisplayID displayID = CGMainDisplayID();
		CVDisplayLinkCreateWithCGDisplay(displayID, &__mpAppData.displayLink);
		CVDisplayLinkSetOutputCallback(__mpAppData.displayLink, DisplayLinkCallback, 0);
		*/

		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		[NSApp activateIgnoringOtherApps:YES];

	}
}}

void mp_terminate()
{
	//TODO: proper app data cleanup (eg delegate, etc)
	if(__mpAppData.init)
	{
		mem_arena_release(&__mpAppData.eventArena);
		__mpAppData = (mp_app_data){0};
	}
}

bool mp_should_quit()
{
	return(__mpAppData.shouldQuit);
}

void mp_do_quit()
{
	__mpAppData.shouldQuit = true;
}

void mp_cancel_quit()
{
	__mpAppData.shouldQuit = false;
}

void mp_request_quit()
{
	__mpAppData.shouldQuit = true;
	mp_event event = {};
	event.type = MP_EVENT_QUIT;
	mp_dispatch_event(&event);
}

void mp_set_cursor(mp_mouse_cursor cursor)
{
	switch(cursor)
	{
		case MP_MOUSE_CURSOR_ARROW:
		{
			__mpAppData.cursor = [NSCursor arrowCursor];
		} break;
		case MP_MOUSE_CURSOR_RESIZE_0:
		{
			__mpAppData.cursor = [[NSCursor class] performSelector:@selector(_windowResizeEastWestCursor)];
		} break;
		case MP_MOUSE_CURSOR_RESIZE_90:
		{
			__mpAppData.cursor = [[NSCursor class] performSelector:@selector(_windowResizeNorthSouthCursor)];
		} break;
		case MP_MOUSE_CURSOR_RESIZE_45:
		{
			__mpAppData.cursor = [[NSCursor class] performSelector:@selector(_windowResizeNorthEastSouthWestCursor)];
		} break;
		case MP_MOUSE_CURSOR_RESIZE_135:
		{
			__mpAppData.cursor = [[NSCursor class] performSelector:@selector(_windowResizeNorthWestSouthEastCursor)];
		} break;
		case MP_MOUSE_CURSOR_TEXT:
		{
			__mpAppData.cursor = [NSCursor IBeamCursor];
		} break;
	}
	[__mpAppData.cursor set];
}

void mp_clipboard_clear()
{@autoreleasepool{
	NSPasteboard* pb = [NSPasteboard generalPasteboard];
	[pb clearContents];
}}

void mp_clipboard_set_string(str8 string)
{@autoreleasepool{

	NSString* nsString = [[NSString alloc] initWithBytes:string.ptr length:string.len encoding:NSUTF8StringEncoding];
	NSPasteboard* pb = [NSPasteboard generalPasteboard];
	[pb writeObjects:[[NSArray alloc] initWithObjects:nsString, nil]];
}}

str8 mp_clipboard_copy_string(str8 backing)
{@autoreleasepool{
	//WARN(martin): maxSize includes space for a null terminator

	NSPasteboard* pb = [NSPasteboard generalPasteboard];
	NSString* nsString = [pb stringForType:NSPasteboardTypeString];
	const char* cString = [nsString UTF8String];
	u32 len = minimum(backing.len-1, strlen(cString)); //length without null terminator
	strncpy(backing.ptr, cString, backing.len-1);
	backing.ptr[len] = '\0';

	str8 result = str8_slice(backing, 0, len);
	return(result);
}}

str8 mp_clipboard_get_string(mem_arena* arena)
{@autoreleasepool{
	//WARN(martin): maxSize includes space for a null terminator

	NSPasteboard* pb = [NSPasteboard generalPasteboard];
	NSString* nsString = [pb stringForType:NSPasteboardTypeString];
	const char* cString = [nsString UTF8String];
	str8 result = str8_push_cstring(arena, cString);
	return(result);
}}

bool mp_clipboard_has_tag(const char* tag)
{@autoreleasepool{

	NSString* tagString = [[NSString alloc] initWithUTF8String: tag];
	NSArray* tagArray = [NSArray arrayWithObjects: tagString, nil];

	NSPasteboard* pb = [NSPasteboard generalPasteboard];
	NSString* available = [pb availableTypeFromArray: tagArray];

	return(available != nil);
}}

void mp_clipboard_set_data_for_tag(const char* tag, str8 string)
{@autoreleasepool{

	NSString* tagString = [[NSString alloc] initWithUTF8String: tag];
	NSArray* tagArray = [NSArray arrayWithObjects: tagString, nil];
	NSData* nsData = [NSData dataWithBytes:string.ptr length:string.len];

	NSPasteboard* pb = [NSPasteboard generalPasteboard];
	[pb addTypes: tagArray owner:nil];
	[pb setData: nsData forType: tagString];
}}

str8 mp_clipboard_get_data_for_tag(mem_arena* arena, const char* tag)
{@autoreleasepool{

	NSString* tagString = [[NSString alloc] initWithUTF8String: tag];

	NSPasteboard* pb = [NSPasteboard generalPasteboard];
	NSData* nsData = [pb dataForType: tagString];
	str8 result = str8_push_buffer(arena, [nsData length], (char*)[nsData bytes]);
	return(result);
}}


//---------------------------------------------------------------
// Window public API
//---------------------------------------------------------------

bool mp_window_handle_is_null(mp_window window)
{
	return(window.h == 0);
}

mp_window mp_window_null_handle()
{
	return((mp_window){.h = 0});
}

/*
//TODO(martin): review include scheme
extern "C" {
	mp_graphics_surface mp_metal_surface_create_for_window_ptr(mp_window_data* window);
	mp_graphics_surface mp_graphics_surface_null_handle();
	mp_graphics_surface mp_graphics_surface_handle_from_ptr(mp_graphics_surface_data* surface);
}
*/

mp_window mp_window_create(mp_rect contentRect, const char* title, mp_window_style style)
{@autoreleasepool{
	mp_window_data* window = mp_window_alloc();
	if(!window)
	{
		LOG_ERROR("Could not allocate window data\n");
		return((mp_window){0});
	}

	window->style = style;
	window->shouldClose = false;
	window->hidden = true;

	u32 styleMask = mp_osx_get_window_style_mask(style);

	NSRect screenRect = [[NSScreen mainScreen] frame];
	NSRect rect = NSMakeRect(contentRect.x,
				 screenRect.size.height - contentRect.y - contentRect.h,
				 contentRect.w,
				 contentRect.h);

	window->nsWindow = [[MPNativeWindow alloc] initWithMPWindow: window contentRect:rect styleMask:styleMask];
	window->nsWindowDelegate = [[MPNativeWindowDelegate alloc] initWithMPWindow:window];

	[window->nsWindow setDelegate:(id)window->nsWindowDelegate];
	[window->nsWindow setTitle:[NSString stringWithUTF8String:title]];

	if(style & MP_WINDOW_STYLE_NO_TITLE)
	{
		[window->nsWindow setOpaque:NO];
		[window->nsWindow setBackgroundColor:[NSColor clearColor]];
		[window->nsWindow setHasShadow:YES];
	}
	if(style & MP_WINDOW_STYLE_FLOAT)
	{
		[window->nsWindow setLevel:NSFloatingWindowLevel];
		[window->nsWindow setHidesOnDeactivate:YES];
	}
	if(style & MP_WINDOW_STYLE_NO_BUTTONS)
	{
		[[window->nsWindow standardWindowButton:NSWindowCloseButton] setHidden:YES];
		[[window->nsWindow standardWindowButton:NSWindowMiniaturizeButton] setHidden:YES];
		[[window->nsWindow standardWindowButton:NSWindowZoomButton] setHidden:YES];
	}

	MPNativeView* view = [[MPNativeView alloc] initWithMPWindow:window];

	[window->nsWindow setContentView:view];
	[window->nsWindow makeFirstResponder:view];
	[window->nsWindow setAcceptsMouseMovedEvents:YES];

	mp_window_update_rect_cache(window);

	mp_window windowHandle = mp_window_handle_from_ptr(window);

	mp_rect mainViewFrame = {0, 0, contentRect.w, contentRect.h};
	window->mainView = mp_view_create(windowHandle, mainViewFrame);

	return(windowHandle);
}//autoreleasepool
}

void mp_window_destroy(mp_window window)
{@autoreleasepool{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		[windowData->nsWindow orderOut:nil];

		[windowData->nsWindow setDelegate:nil];
		[windowData->nsWindowDelegate release];
		windowData->nsWindowDelegate = nil;

		[windowData->nsView release];
		windowData->nsView = nil;

		[windowData->nsWindow close]; //also release the window

		mp_window_recycle_ptr(windowData);
	}
} // autoreleasepool
}

bool mp_window_should_close(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		return(windowData->shouldClose);
	}
	else
	{
		return(false);
	}
}

void mp_window_cancel_close(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		windowData->shouldClose = false;
	}
}

void mp_window_request_close(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		[windowData->nsWindow close];
		//NOTE(martin): this will call our window delegate willClose method
	}
}

void* mp_window_native_pointer(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		return((__bridge void*)windowData->nsWindow);
	}
	else
	{
		return(0);
	}
}

void mp_window_center(mp_window window)
{@autoreleasepool{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		[windowData->nsWindow center];
	}
}}

bool mp_window_is_hidden(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		return(windowData->hidden);
	}
	else
	{
		return(false);
	}
}

bool mp_window_is_focused(mp_window window)
{@autoreleasepool{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		return([windowData->nsWindow isKeyWindow]);
	}
	else
	{
		return(false);
	}
}}

void mp_window_hide(mp_window window)
{@autoreleasepool{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		windowData->hidden = true;
		[windowData->nsWindow orderOut:nil];
	}
}}

void mp_window_focus(mp_window window)
{@autoreleasepool{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		[windowData->nsWindow makeKeyWindow];
	}
}}

void mp_window_send_to_back(mp_window window)
{@autoreleasepool{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		[windowData->nsWindow orderBack:nil];
	}
}}

void mp_window_bring_to_front(mp_window window)
{@autoreleasepool{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		windowData->hidden = false;
		[windowData->nsWindow orderFront:nil];
	}
}}

void mp_window_bring_to_front_and_focus(mp_window window)
{
	mp_window_bring_to_front(window);
	mp_window_focus(window);
}


mp_rect mp_window_content_rect_for_frame_rect(mp_rect frameRect, mp_window_style style)
{@autoreleasepool{
	u32 mask = mp_osx_get_window_style_mask(style);
	mp_rect nativeFrame = mp_user_to_osx_screen_rect(frameRect);
	NSRect frame = NSMakeRect(nativeFrame.x, nativeFrame.y, nativeFrame.w, nativeFrame.h);
	NSRect content = [NSWindow contentRectForFrameRect:frame styleMask:mask];
	mp_rect result = {content.origin.x, content.origin.y, content.size.width, content.size.height};
	result = mp_osx_to_user_screen_rect(result);
	return(result);
}}

mp_rect mp_window_frame_rect_for_content_rect(mp_rect contentRect, mp_window_style style)
{@autoreleasepool{
	uint32 mask = mp_osx_get_window_style_mask(style);
	mp_rect nativeContent = mp_user_to_osx_screen_rect(contentRect);
	NSRect content = NSMakeRect(nativeContent.x, nativeContent.y, nativeContent.w, nativeContent.h);
	NSRect frame = [NSWindow frameRectForContentRect:content styleMask:mask];
	mp_rect result = {frame.origin.x, frame.origin.y, frame.size.width, frame.size.height};
	result = mp_osx_to_user_screen_rect(result);
	return(result);
}}

mp_rect mp_window_get_content_rect(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		return(windowData->contentRect);
	}
	else
	{
		return((mp_rect){});
	}
}

mp_rect mp_window_get_absolute_content_rect(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		mp_rect rect = windowData->contentRect;
		rect.x += windowData->frameRect.x;
		rect.y += windowData->frameRect.y;
		return(rect);
	}
	else
	{
		return((mp_rect){});
	}
}

mp_rect mp_window_get_frame_rect(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		return(windowData->frameRect);
	}
	else
	{
		return((mp_rect){});
	}
}

void mp_window_set_content_rect(mp_window window, mp_rect contentRect)
{@autoreleasepool{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		u32 mask = mp_osx_get_window_style_mask(windowData->style);

		mp_rect nativeRect = mp_user_to_osx_screen_rect(contentRect);
		NSRect content = NSMakeRect(nativeRect.x, nativeRect.y, nativeRect.w, nativeRect.h);
		NSRect frame = [NSWindow frameRectForContentRect:content styleMask:mask];

		[windowData->nsWindow setFrame:frame display:YES];

		mp_window_update_rect_cache(windowData);
	}
}}
void mp_window_set_frame_rect(mp_window window, mp_rect frameRect)
{@autoreleasepool{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		mp_rect nativeRect = mp_user_to_osx_screen_rect(frameRect);
		NSRect frame = NSMakeRect(nativeRect.x, nativeRect.y, nativeRect.w, nativeRect.h);
		[windowData->nsWindow setFrame:frame display:YES];

		mp_window_update_rect_cache(windowData);
		NSRect contentRect = [[windowData->nsWindow contentView] frame];
	}
}}

void mp_window_set_frame_size(mp_window window, int width, int height)
{
	mp_rect frame = mp_window_get_frame_rect(window);
	frame.w = width;
	frame.h = height;
	mp_window_set_frame_rect(window, frame);
}

void mp_window_set_content_size(mp_window window, int width, int height)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		mp_rect frame = windowData->frameRect;
		mp_rect content = mp_window_content_rect_for_frame_rect(frame, windowData->style);
		content.w = width;
		content.h = height;
		frame = mp_window_frame_rect_for_content_rect(content, windowData->style);
		mp_window_set_frame_rect(window, frame);
	}
}

//--------------------------------------------------------------------
// view management
//--------------------------------------------------------------------

mp_view mp_view_nil()
{
	return((mp_view){0});
}

bool mp_view_is_nil(mp_view view)
{
	return(!view.h);
}

mp_view mp_view_create(mp_window windowHandle, mp_rect frame)
{@autoreleasepool{
	mp_window_data* window = mp_window_ptr_from_handle(windowHandle);
	if(!window)
	{
		LOG_ERROR("Can't create view for nil window\n");
		return(mp_view_nil());
	}

	mp_view_data* view = mp_view_alloc();
	if(!view)
	{
		LOG_ERROR("Could not allocate view data\n");
		return(mp_view_nil());
	}

	view->window = windowHandle;

	NSRect nsFrame = {{frame.x, frame.y}, {frame.w, frame.h}};
	view->nsView = [[NSView alloc] initWithFrame: nsFrame];
	[view->nsView setWantsLayer:YES];

	[[window->nsWindow contentView] addSubview: view->nsView];

	return(mp_view_handle_from_ptr(view));
}}

void mp_view_destroy(mp_view viewHandle)
{@autoreleasepool{
	mp_view_data* view = mp_view_ptr_from_handle(viewHandle);
	if(!view)
	{
		return;
	}

	mp_window_data* window = mp_window_ptr_from_handle(view->window);
	if(!window)
	{
		return;
	}

	[view->nsView removeFromSuperview];

	mp_view_recycle_ptr(view);
}}

void mp_view_set_frame(mp_view viewHandle, mp_rect frame)
{
	mp_view_data* view = mp_view_ptr_from_handle(viewHandle);
	if(!view)
	{
		return;
	}

	NSRect nsFrame = {{frame.x, frame.y}, {frame.w, frame.h}};
	[view->nsView setFrame: nsFrame];
	mg_surface_resize(view->surface, frame.w, frame.h);
}

//--------------------------------------------------------------------
// Main loop throttle
//--------------------------------------------------------------------

void mp_set_target_fps(u32 fps)
{
	__mpAppData.frameStats.targetFramePeriod = 1./(f64)fps;
	__mpAppData.frameStats.workTime = 0;
	__mpAppData.frameStats.remainingTime = 0;

	if(__mpAppData.frameTimer)
	{
		[__mpAppData.frameTimer invalidate];
	}

	__mpAppData.frameTimer = [NSTimer timerWithTimeInterval: __mpAppData.frameStats.targetFramePeriod
	                          target: [NSApp delegate]
				  selector:@selector(timerElapsed:)
				  userInfo:nil
				  repeats:YES];

	[[NSRunLoop currentRunLoop] addTimer:__mpAppData.frameTimer forMode:NSRunLoopCommonModes];
}
/*
void mp_begin_frame()
{
	__mpAppData.frameStats.start = mp_get_elapsed_seconds();

	LOG_DEBUG("workTime = %.6f (%.6f fps), remaining = %.6f\n",
	             __mpAppData.frameStats.workTime,
	             1/__mpAppData.frameStats.workTime,
	             __mpAppData.frameStats.remainingTime);

}

void mp_end_frame()
{
	__mpAppData.frameStats.workTime = mp_get_elapsed_seconds() - __mpAppData.frameStats.start;
	__mpAppData.frameStats.remainingTime = __mpAppData.frameStats.targetFramePeriod - __mpAppData.frameStats.workTime;

	while(__mpAppData.frameStats.remainingTime > 100e-9)
	{
		if(__mpAppData.frameStats.remainingTime > 10e-6)
		{
			mp_sleep_nanoseconds(__mpAppData.frameStats.remainingTime*0.8*1e9);
		}
		__mpAppData.frameStats.workTime = mp_get_elapsed_seconds() - __mpAppData.frameStats.start;
		__mpAppData.frameStats.remainingTime = __mpAppData.frameStats.targetFramePeriod - __mpAppData.frameStats.workTime;
	}
}
*/

//--------------------------------------------------------------------
// Events handling
//--------------------------------------------------------------------

void mp_set_live_resize_callback(mp_live_resize_callback callback, void* data)
{
	__mpAppData.liveResizeCallback = callback;
	__mpAppData.liveResizeData = data;
}


void mp_set_event_callback(mp_event_callback callback, void* data)
{
	__mpAppData.eventCallback = callback;
	__mpAppData.eventData = data;
}

void mp_pump_events(f64 timeout)
{
	__mpAppData.inputState.frameCounter++;

	@autoreleasepool
	{
		bool accumulate = false;
		NSDate* date = 0;
		if(timeout > 0)
		{
			date = [NSDate dateWithTimeIntervalSinceNow: (double) timeout];
		}
		else if(timeout == 0)
		{
			date = [NSDate distantPast];
			accumulate = true;
		}
		else
		{
			date = [NSDate distantFuture];
		}

		for(;;)
		{
			NSEvent* event = [NSApp nextEventMatchingMask: NSEventMaskAny
						untilDate:date
						inMode: NSDefaultRunLoopMode
						dequeue: YES];

			if(event != nil)
			{
				[NSApp sendEvent:event];

				if(!accumulate)
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
	}
}

bool mp_next_event(mp_event* event)
{
	//NOTE pop and return event from queue
	if(ringbuffer_read_available(&__mpAppData.eventQueue) >= sizeof(mp_event))
	{
		u64 read = ringbuffer_read(&__mpAppData.eventQueue, sizeof(mp_event), (u8*)event);
		DEBUG_ASSERT(read == sizeof(mp_event));
		return(true);
	}
	else
	{
		return(false);
	}
}

void mp_run_loop()
{@autoreleasepool {

	//CVDisplayLinkStart(__mpAppData.displayLink);

	while(!__mpAppData.shouldQuit)
	{
		mp_event event;
		while(mp_next_event(&event))
		{
			//send pending event that might have accumulated before we started run loop
			if(__mpAppData.eventCallback)
			{
				__mpAppData.eventCallback(event, __mpAppData.eventData);
			}
		}

		NSEvent* nsEvent = [NSApp nextEventMatchingMask: NSEventMaskAny
					untilDate:[NSDate distantFuture]
					inMode: NSDefaultRunLoopMode
					dequeue: YES];

		if(nsEvent != nil)
		{
			[NSApp sendEvent:nsEvent];
		}
	}

	//CVDisplayLinkStop(__mpAppData.displayLink);
}}

void mp_end_input_frame()
{
	//////////////////////////////////////////////////////////////////////////////////////////////
	//TODO: make sure we call arena clear once per event frame, even when using runloop etc...
	//////////////////////////////////////////////////////////////////////////////////////////////
	__mpAppData.inputState.frameCounter++;
	mem_arena_clear(&__mpAppData.eventArena);
}

//--------------------------------------------------------------------
// app resources
//--------------------------------------------------------------------

#import<Foundation/Foundation.h>
#include<libgen.h>

str8 mp_app_get_resource_path(mem_arena* arena, const char* name)
{
	str8_list list = {};
	mem_arena* scratch = mem_scratch();

	str8 executablePath = mp_app_get_executable_path(scratch);
	str8 dirPath = mp_path_directory(executablePath);

	str8_list_push(scratch, &list, dirPath);
	str8_list_push(scratch, &list, str8_lit("/"));
	str8_list_push(scratch, &list, str8_push_cstring(scratch, name));
	str8 path = str8_list_join(scratch, list);
	char* pathCString = str8_to_cstring(scratch, path);
	char* buffer = mem_arena_alloc_array(scratch, char, path.len+1);
	char* real = realpath(pathCString, buffer);

	str8 result = str8_push_cstring(arena, real);
	return(result);
}


#include<mach-o/dyld.h>
str8 mp_app_get_executable_path(mem_arena* arena)
{@autoreleasepool{
	str8 result = {};
	u32 size = 0;
	_NSGetExecutablePath(0, &size);
	result.len = size;
	result.ptr = mem_arena_alloc_array(arena, char, result.len);
	_NSGetExecutablePath(result.ptr, &size);
	return(result);
}}

//--------------------------------------------------------------------
// system dialogs windows
//--------------------------------------------------------------------

str8 mp_open_dialog(mem_arena* arena,
                           const char* title,
                           const char* defaultPath,
                           int filterCount,
                           const char** filters,
                           bool directory)
{
	@autoreleasepool
	{
		NSWindow *keyWindow = [NSApp keyWindow];

		NSOpenPanel* dialog = [NSOpenPanel openPanel] ;
		[dialog setLevel:CGShieldingWindowLevel()];

		if(filterCount)
		{
	 		NSMutableArray * fileTypesArray = [NSMutableArray array];
			for(int i=0; i < filterCount; i++)
			{
				NSString * filt = [NSString stringWithUTF8String:filters[i]];
				[fileTypesArray addObject:filt];
			}
			[dialog setAllowedFileTypes:fileTypesArray];
		}
		// Enable options in the dialog.
		if(directory)
		{
			[dialog setCanChooseDirectories:YES];
		}
		else
		{
			[dialog setCanChooseFiles:YES];
		}


		[dialog setAllowsMultipleSelection:FALSE];
		NSString* nsPath =  [[NSString stringWithUTF8String:defaultPath?defaultPath:"~"] stringByExpandingTildeInPath];
		[dialog setDirectoryURL:[NSURL fileURLWithPath:nsPath]];

		// Display the dialog box. If the OK pressed,
		// process the files.

		if( [dialog runModal] == NSModalResponseOK )
		{
			// Gets list of all files selected
			NSArray *files = [dialog URLs];
			//TODO: Loop through the files and process them.

			const char* result = [[[files objectAtIndex:0] path] UTF8String];

			str8 path = str8_push_cstring(arena, result);
			[keyWindow makeKeyWindow];

			return(path);
		}
		else
		{
			[keyWindow makeKeyWindow];
			return((str8){0, 0});
		}
	}
}


str8 mp_save_dialog(mem_arena* arena,
                           const char* title,
                           const char* defaultPath,
                           int filterCount,
                           const char** filters)
{
	@autoreleasepool
	{
		NSWindow *keyWindow = [NSApp keyWindow];

		NSSavePanel* dialog = [NSSavePanel savePanel] ;
		[dialog setLevel:CGShieldingWindowLevel()];

		if(filterCount)
		{
		 	NSMutableArray * fileTypesArray = [NSMutableArray array];
			for(int i=0; i < filterCount; i++)
			{
				NSString * filt = [NSString stringWithUTF8String:filters[i]];
				[fileTypesArray addObject:filt];
			}

			// Enable options in the dialog.
			[dialog setAllowedFileTypes:fileTypesArray];
		}
		NSString* nsPath =  [[NSString stringWithUTF8String:defaultPath?defaultPath:"~"] stringByExpandingTildeInPath];
		[dialog setDirectoryURL:[NSURL fileURLWithPath:nsPath]];

		// Display the dialog box. If the OK pressed,
		// process the files.

		if( [dialog runModal] == NSModalResponseOK )
		{
			// Gets list of all files selected
			NSURL *files = [dialog URL];
			// Loop through the files and process them.

			const char* result = [[files path] UTF8String];

			str8 path = str8_push_cstring(arena, result);
			[keyWindow makeKeyWindow];
			return(path);
		}
		else
		{
			[keyWindow makeKeyWindow];
			return((str8){0, 0});
		}
	}
}

int mp_alert_popup(const char* title,
                   const char* message,
                   uint32 count,
                   const char** options)
{
	@autoreleasepool
	{
		NSWindow *keyWindow = [NSApp keyWindow];

		NSAlert* alert = [[NSAlert alloc] init];
		NSString* string;
		for(int i=count-1;i>=0;i--)
		{
			string = [[NSString alloc] initWithUTF8String:options[i]];
			[alert addButtonWithTitle:string];
			[string release];
		}
		string = [[NSString alloc] initWithUTF8String:message];
		[alert setMessageText:string];
		[string release];

		[alert setAlertStyle:NSAlertStyleWarning];
		int result = count - ([alert runModal]-NSAlertFirstButtonReturn) - 1;
		printf("result = %i, NSAlertFirstButtonReturn = %li\n", result, (long)NSAlertFirstButtonReturn);
		[keyWindow makeKeyWindow];
		return(result);
	}
}


//--------------------------------------------------------------------
// file system stuff... //TODO: move elsewhere
//--------------------------------------------------------------------

int mp_file_move(str8 from, str8 to)
{@autoreleasepool{
	NSString* nsFrom = [[NSString alloc] initWithBytes:from.ptr length:from.len encoding: NSUTF8StringEncoding];
	NSString* nsTo = [[NSString alloc] initWithBytes:to.ptr length:to.len encoding: NSUTF8StringEncoding];
	NSError* err;
	if([[NSFileManager defaultManager] moveItemAtPath:nsFrom toPath:nsTo error:&err] == YES)
	{
		return(0);
	}
	else
	{
		return(-1);
	}
}}

int mp_file_remove(str8 path)
{@autoreleasepool{
	NSString* nsPath = [[NSString alloc] initWithBytes:path.ptr length:path.len encoding: NSUTF8StringEncoding];
	NSError* err;
	if([[NSFileManager defaultManager] removeItemAtPath:nsPath error:&err] == YES)
	{
		return(0);
	}
	else
	{
		return(-1);
	}
}}

int mp_directory_create(str8 path)
{@autoreleasepool{

	NSString* nsPath = [[NSString alloc] initWithBytes:path.ptr length:path.len encoding: NSUTF8StringEncoding];
	NSError* err;
	if([[NSFileManager defaultManager] createDirectoryAtPath: nsPath
	                                withIntermediateDirectories: YES
	                                attributes:nil
	                                error:&err] == YES)
	{
		return(0);
	}
	else
	{
		return(-1);
	}
}}

#undef LOG_SUBSYSTEM
