//*****************************************************************
//
//	$file: osx_app.m $
//	$author: Martin Fouilleul $
//	$date: 16/05/2020 $
//	$revision: $
//	$note: (C) 2020 by Martin Fouilleul - all rights reserved $
//
//*****************************************************************

#import <QuartzCore/QuartzCore.h> //CATransaction

#include<stdlib.h> // malloc/free

#include"lists.h"
#include"ringbuffer.h"
#include"memory.h"
#include"macro_helpers.h"
#include"platform_clock.h"
#include"graphics_internal.h"

#include"mp_app.c"

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
		NSRect frame = [window->osx.nsWindow frame];
		window->frameRect = mp_osx_to_user_screen_rect((mp_rect){frame.origin.x, frame.origin.y, frame.size.width, frame.size.height});

		const NSRect contentRect = [[window->osx.nsWindow contentView] frame];

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

static void mp_init_osx_keys()
{
	memset(__mpApp.keyCodes, MP_KEY_UNKNOWN, 256*sizeof(int));

	__mpApp.keyCodes[0x1D] = MP_KEY_0;
	__mpApp.keyCodes[0x12] = MP_KEY_1;
	__mpApp.keyCodes[0x13] = MP_KEY_2;
	__mpApp.keyCodes[0x14] = MP_KEY_3;
	__mpApp.keyCodes[0x15] = MP_KEY_4;
	__mpApp.keyCodes[0x17] = MP_KEY_5;
	__mpApp.keyCodes[0x16] = MP_KEY_6;
	__mpApp.keyCodes[0x1A] = MP_KEY_7;
	__mpApp.keyCodes[0x1C] = MP_KEY_8;
	__mpApp.keyCodes[0x19] = MP_KEY_9;
	__mpApp.keyCodes[0x00] = MP_KEY_A;
	__mpApp.keyCodes[0x0B] = MP_KEY_B;
	__mpApp.keyCodes[0x08] = MP_KEY_C;
	__mpApp.keyCodes[0x02] = MP_KEY_D;
	__mpApp.keyCodes[0x0E] = MP_KEY_E;
	__mpApp.keyCodes[0x03] = MP_KEY_F;
	__mpApp.keyCodes[0x05] = MP_KEY_G;
	__mpApp.keyCodes[0x04] = MP_KEY_H;
	__mpApp.keyCodes[0x22] = MP_KEY_I;
	__mpApp.keyCodes[0x26] = MP_KEY_J;
	__mpApp.keyCodes[0x28] = MP_KEY_K;
	__mpApp.keyCodes[0x25] = MP_KEY_L;
	__mpApp.keyCodes[0x2E] = MP_KEY_M;
	__mpApp.keyCodes[0x2D] = MP_KEY_N;
	__mpApp.keyCodes[0x1F] = MP_KEY_O;
	__mpApp.keyCodes[0x23] = MP_KEY_P;
	__mpApp.keyCodes[0x0C] = MP_KEY_Q;
	__mpApp.keyCodes[0x0F] = MP_KEY_R;
	__mpApp.keyCodes[0x01] = MP_KEY_S;
	__mpApp.keyCodes[0x11] = MP_KEY_T;
	__mpApp.keyCodes[0x20] = MP_KEY_U;
	__mpApp.keyCodes[0x09] = MP_KEY_V;
	__mpApp.keyCodes[0x0D] = MP_KEY_W;
	__mpApp.keyCodes[0x07] = MP_KEY_X;
	__mpApp.keyCodes[0x10] = MP_KEY_Y;
	__mpApp.keyCodes[0x06] = MP_KEY_Z;

	__mpApp.keyCodes[0x27] = MP_KEY_APOSTROPHE;
	__mpApp.keyCodes[0x2A] = MP_KEY_BACKSLASH;
	__mpApp.keyCodes[0x2B] = MP_KEY_COMMA;
	__mpApp.keyCodes[0x18] = MP_KEY_EQUAL;
	__mpApp.keyCodes[0x32] = MP_KEY_GRAVE_ACCENT;
	__mpApp.keyCodes[0x21] = MP_KEY_LEFT_BRACKET;
	__mpApp.keyCodes[0x1B] = MP_KEY_MINUS;
	__mpApp.keyCodes[0x2F] = MP_KEY_PERIOD;
	__mpApp.keyCodes[0x1E] = MP_KEY_RIGHT_BRACKET;
	__mpApp.keyCodes[0x29] = MP_KEY_SEMICOLON;
	__mpApp.keyCodes[0x2C] = MP_KEY_SLASH;
	__mpApp.keyCodes[0x0A] = MP_KEY_WORLD_1;

	__mpApp.keyCodes[0x33] = MP_KEY_BACKSPACE;
	__mpApp.keyCodes[0x39] = MP_KEY_CAPS_LOCK;
	__mpApp.keyCodes[0x75] = MP_KEY_DELETE;
	__mpApp.keyCodes[0x7D] = MP_KEY_DOWN;
	__mpApp.keyCodes[0x77] = MP_KEY_END;
	__mpApp.keyCodes[0x24] = MP_KEY_ENTER;
	__mpApp.keyCodes[0x35] = MP_KEY_ESCAPE;
	__mpApp.keyCodes[0x7A] = MP_KEY_F1;
	__mpApp.keyCodes[0x78] = MP_KEY_F2;
	__mpApp.keyCodes[0x63] = MP_KEY_F3;
	__mpApp.keyCodes[0x76] = MP_KEY_F4;
	__mpApp.keyCodes[0x60] = MP_KEY_F5;
	__mpApp.keyCodes[0x61] = MP_KEY_F6;
	__mpApp.keyCodes[0x62] = MP_KEY_F7;
	__mpApp.keyCodes[0x64] = MP_KEY_F8;
	__mpApp.keyCodes[0x65] = MP_KEY_F9;
	__mpApp.keyCodes[0x6D] = MP_KEY_F10;
	__mpApp.keyCodes[0x67] = MP_KEY_F11;
	__mpApp.keyCodes[0x6F] = MP_KEY_F12;
	__mpApp.keyCodes[0x69] = MP_KEY_F13;
	__mpApp.keyCodes[0x6B] = MP_KEY_F14;
	__mpApp.keyCodes[0x71] = MP_KEY_F15;
	__mpApp.keyCodes[0x6A] = MP_KEY_F16;
	__mpApp.keyCodes[0x40] = MP_KEY_F17;
	__mpApp.keyCodes[0x4F] = MP_KEY_F18;
	__mpApp.keyCodes[0x50] = MP_KEY_F19;
	__mpApp.keyCodes[0x5A] = MP_KEY_F20;
	__mpApp.keyCodes[0x73] = MP_KEY_HOME;
	__mpApp.keyCodes[0x72] = MP_KEY_INSERT;
	__mpApp.keyCodes[0x7B] = MP_KEY_LEFT;
	__mpApp.keyCodes[0x3A] = MP_KEY_LEFT_ALT;
	__mpApp.keyCodes[0x3B] = MP_KEY_LEFT_CONTROL;
	__mpApp.keyCodes[0x38] = MP_KEY_LEFT_SHIFT;
	__mpApp.keyCodes[0x37] = MP_KEY_LEFT_SUPER;
	__mpApp.keyCodes[0x6E] = MP_KEY_MENU;
	__mpApp.keyCodes[0x47] = MP_KEY_NUM_LOCK;
	__mpApp.keyCodes[0x79] = MP_KEY_PAGE_DOWN;
	__mpApp.keyCodes[0x74] = MP_KEY_PAGE_UP;
	__mpApp.keyCodes[0x7C] = MP_KEY_RIGHT;
	__mpApp.keyCodes[0x3D] = MP_KEY_RIGHT_ALT;
	__mpApp.keyCodes[0x3E] = MP_KEY_RIGHT_CONTROL;
	__mpApp.keyCodes[0x3C] = MP_KEY_RIGHT_SHIFT;
	__mpApp.keyCodes[0x36] = MP_KEY_RIGHT_SUPER;
	__mpApp.keyCodes[0x31] = MP_KEY_SPACE;
	__mpApp.keyCodes[0x30] = MP_KEY_TAB;
	__mpApp.keyCodes[0x7E] = MP_KEY_UP;

	__mpApp.keyCodes[0x52] = MP_KEY_KP_0;
	__mpApp.keyCodes[0x53] = MP_KEY_KP_1;
	__mpApp.keyCodes[0x54] = MP_KEY_KP_2;
	__mpApp.keyCodes[0x55] = MP_KEY_KP_3;
	__mpApp.keyCodes[0x56] = MP_KEY_KP_4;
	__mpApp.keyCodes[0x57] = MP_KEY_KP_5;
	__mpApp.keyCodes[0x58] = MP_KEY_KP_6;
	__mpApp.keyCodes[0x59] = MP_KEY_KP_7;
	__mpApp.keyCodes[0x5B] = MP_KEY_KP_8;
	__mpApp.keyCodes[0x5C] = MP_KEY_KP_9;
	__mpApp.keyCodes[0x45] = MP_KEY_KP_ADD;
	__mpApp.keyCodes[0x41] = MP_KEY_KP_DECIMAL;
	__mpApp.keyCodes[0x4B] = MP_KEY_KP_DIVIDE;
	__mpApp.keyCodes[0x4C] = MP_KEY_KP_ENTER;
	__mpApp.keyCodes[0x51] = MP_KEY_KP_EQUAL;
	__mpApp.keyCodes[0x43] = MP_KEY_KP_MULTIPLY;
	__mpApp.keyCodes[0x4E] = MP_KEY_KP_SUBTRACT;

	memset(__mpApp.nativeKeys, 0, sizeof(int)*MP_KEY_COUNT);
	for(int nativeKey=0; nativeKey<256; nativeKey++)
	{
		mp_key_code mpKey = __mpApp.keyCodes[nativeKey];
		if(mpKey)
		{
			__mpApp.nativeKeys[mpKey] = nativeKey;
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
		return(__mpApp.keyCodes[nsCode]);
	}
}

static mp_keymod_flags mp_convert_osx_mods(NSUInteger nsFlags)
{
	mp_keymod_flags mods = MP_KEYMOD_NONE;
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
	if(__mpApp.osx.kbLayoutInputSource)
	{
		CFRelease(__mpApp.osx.kbLayoutInputSource);
		__mpApp.osx.kbLayoutInputSource = 0;
		__mpApp.osx.kbLayoutUnicodeData = nil;
	}

	__mpApp.osx.kbLayoutInputSource = TISCopyCurrentKeyboardLayoutInputSource();
	if(!__mpApp.osx.kbLayoutInputSource)
	{
		LOG_ERROR("Failed to load keyboard layout input source");
	}

	__mpApp.osx.kbLayoutUnicodeData = TISGetInputSourceProperty(__mpApp.osx.kbLayoutInputSource,
	                                                            kTISPropertyUnicodeKeyLayoutData);
	if(!__mpApp.osx.kbLayoutUnicodeData)
	{
		LOG_ERROR("Failed to load keyboard layout unicode data");
	}

	memset(__mpApp.keyLabels, 0, sizeof(mp_key_utf8)*MP_KEY_COUNT);

	for(int key=0; key<MP_KEY_COUNT; key++)
	{
		//TODO: check that the key is printable
		int nativeKey = __mpApp.nativeKeys[key];

		UInt32 deadKeyState = 0;
		UniChar characters[4];
		UniCharCount characterCount = 0;

		if(UCKeyTranslate((UCKeyboardLayout*)[(NSData*) __mpApp.osx.kbLayoutUnicodeData bytes],
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
			__mpApp.keyLabels[key].labelLen = 0;
		}
		else
		{
			NSString* nsString = [[NSString alloc] initWithCharacters: characters length: characterCount];
			const char* cstring = [nsString UTF8String];
			u32 len = strlen(cstring);
			__mpApp.keyLabels[key].labelLen = minimum(len, 8);
			memcpy(__mpApp.keyLabels[key].label, cstring, __mpApp.keyLabels[key].labelLen);
		}
	}
}

str8 mp_key_to_label(mp_key_code key)
{
	mp_key_utf8* keyInfo = &(__mpApp.keyLabels[key]);
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


@interface MPNativeWindow : NSWindow
{
	mp_window_data* mpWindow;
}
- (id)initWithMPWindow:(mp_window_data*) window contentRect:(NSRect) rect styleMask:(uint32) style;
@end


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
	__mpApp.osx.kbLayoutListener = [[MPKeyboardLayoutListener alloc] init];
	[[NSDistributedNotificationCenter defaultCenter]
		addObserver: __mpApp.osx.kbLayoutListener
		selector: @selector(selectedKeyboardInputSourceChanged:)
		name:(__bridge NSString*)kTISNotifySelectedKeyboardInputSourceChanged
		object:nil];
}

static void mp_update_key_mods(mp_keymod_flags mods)
{
	__mpApp.inputState.keyboard.mods = mods;
}

//---------------------------------------------------------------
// Application and app delegate
//---------------------------------------------------------------

@interface MPApplication : NSApplication
@end

@implementation MPApplication
-(void)noOpThread:(id)object
{}

//This is necessary in order to receive keyUp events when we have a key combination with Cmd.
- (void)sendEvent:(NSEvent *)event {
    if ([event type] == NSEventTypeKeyUp && ([event modifierFlags] & NSEventModifierFlagCommand))
        [[self keyWindow] sendEvent:event];
    else
        [super sendEvent:event];
}

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

	__mpApp.shouldQuit = true;
	mp_event event = {};
	event.type = MP_EVENT_QUIT;
	mp_queue_event(&event);

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
	mp_queue_event(&event);
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
	event.path = str8_push_cstring(&__mpApp.eventArena, [filename UTF8String]);

	mp_queue_event(&event);
	return(YES);
}

- (void)handleAppleEvent:(NSAppleEventDescriptor*)appleEvent withReplyEvent:(NSAppleEventDescriptor*)replyEvent
{
	NSString* nsPath = [[appleEvent paramDescriptorForKeyword:keyDirectObject] stringValue];

	mp_event event = {};
	event.window = (mp_window){0};
	event.type = MP_EVENT_PATHDROP;
	event.path = str8_push_cstring(&__mpApp.eventArena, [nsPath UTF8String]);

	mp_queue_event(&event);
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

	mp_queue_event(&event);
}

- (void)windowDidResignKey:(NSNotification*)notification
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(mpWindow);
	event.type = MP_EVENT_WINDOW_UNFOCUS;

	mp_queue_event(&event);
}

- (void)windowDidMove:(NSNotification *)notification
{
	const NSRect contentRect = [[mpWindow->osx.nsWindow contentView] frame];

	mp_window_update_rect_cache(mpWindow);

	mp_event event = {};
	event.window = mp_window_handle_from_ptr(mpWindow);
	event.type = MP_EVENT_WINDOW_MOVE;
	event.frame.rect.x = contentRect.origin.x;
	event.frame.rect.y = contentRect.origin.y;
	event.frame.rect.w = contentRect.size.width;
	event.frame.rect.h = contentRect.size.height;

	mp_queue_event(&event);
}

- (void)windowDidResize:(NSNotification *)notification
{
	const NSRect contentRect = [[mpWindow->osx.nsWindow contentView] frame];

	mp_window_update_rect_cache(mpWindow);

	mp_event event = {};
	event.window = mp_window_handle_from_ptr(mpWindow);
	event.type = MP_EVENT_WINDOW_RESIZE;
	event.frame.rect.x = contentRect.origin.x;
	event.frame.rect.y = contentRect.origin.y;
	event.frame.rect.w = contentRect.size.width;
	event.frame.rect.h = contentRect.size.height;

	if(__mpApp.liveResizeCallback)
	{
		__mpApp.liveResizeCallback(event, __mpApp.liveResizeData);
	}

	//TODO: also ensure we don't overflow the queue during live resize...
	mp_queue_event(&event);
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
	mpWindow->osx.nsWindow = nil;
	[mpWindow->osx.nsView release];
	mpWindow->osx.nsView = nil;
	[mpWindow->osx.nsWindowDelegate release];
	mpWindow->osx.nsWindowDelegate = nil;

	mp_window_recycle_ptr(mpWindow);
}

- (BOOL)windowShouldClose:(id)sender
{
	mpWindow->shouldClose = true;

	mp_event event = {};
	event.window = mp_window_handle_from_ptr(mpWindow);
	event.type = MP_EVENT_WINDOW_CLOSE;

	mp_queue_event(&event);

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
		mpWindow->osx.nsView = self;
		[mpWindow->osx.nsView setWantsLayer:YES];
		mpWindow->osx.nsView.layerContentsRedrawPolicy = NSViewLayerContentsRedrawDuringViewResize;

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
		[window->osx.nsWindow invalidateShadow];
	}
}

- (BOOL)acceptsFirstReponder
{
	return(YES);
}

- (void)cursorUpdate:(NSEvent*)event
{
	if(__mpApp.osx.cursor)
	{
		[__mpApp.osx.cursor set];
	}
	else
	{
		[[NSCursor arrowCursor] set];
	}
}

static void mp_process_mouse_button(NSEvent* nsEvent, mp_window_data* window, mp_mouse_button button, mp_key_action action)
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_BUTTON;
	event.key.action = action;
	event.key.code = button;
	event.key.mods = mp_convert_osx_mods([nsEvent modifierFlags]);
	event.key.clickCount = [nsEvent clickCount];

	mp_key_state* keyState = &__mpApp.inputState.mouse.buttons[event.key.code];
	mp_update_key_state(keyState, action);
	if(action == MP_KEY_PRESS)
	{
		if(event.key.clickCount >= 1)
		{
			keyState->sysClicked = true;
		}
		if(event.key.clickCount >= 2)
		{
			keyState->sysDoubleClicked = true;
		}
	}
	mp_queue_event(&event);
}

- (void)mouseDown:(NSEvent *)nsEvent
{
	mp_process_mouse_button(nsEvent, window, MP_MOUSE_LEFT, MP_KEY_PRESS);
	[window->osx.nsWindow makeFirstResponder:self];
}

- (void)mouseUp:(NSEvent*)nsEvent
{
	mp_process_mouse_button(nsEvent, window, MP_MOUSE_LEFT, MP_KEY_RELEASE);
}

- (void)rightMouseDown:(NSEvent*)nsEvent
{
	mp_process_mouse_button(nsEvent, window, MP_MOUSE_RIGHT, MP_KEY_PRESS);
}

- (void)rightMouseUp:(NSEvent*)nsEvent
{
	mp_process_mouse_button(nsEvent, window, MP_MOUSE_RIGHT, MP_KEY_RELEASE);
}

- (void)otherMouseDown:(NSEvent*)nsEvent
{
	mp_process_mouse_button(nsEvent, window, [nsEvent buttonNumber], MP_KEY_PRESS);
}

- (void)otherMouseUp:(NSEvent*)nsEvent
{
	mp_process_mouse_button(nsEvent, window, [nsEvent buttonNumber], MP_KEY_RELEASE);
}

- (void)mouseDragged:(NSEvent*)nsEvent
{
	[self mouseMoved:nsEvent];
}

- (void)mouseMoved:(NSEvent*)nsEvent
{
	NSPoint p = [self convertPoint:[nsEvent locationInWindow] fromView:nil];

	NSRect frame = [[window->osx.nsWindow contentView] frame];
	mp_event event = {};
	event.type = MP_EVENT_MOUSE_MOVE;
	event.window = mp_window_handle_from_ptr(window);
	event.move.x = p.x;
	event.move.y = p.y;
	event.move.deltaX = [nsEvent deltaX];
	event.move.deltaY = -[nsEvent deltaY];
	event.move.mods = mp_convert_osx_mods([nsEvent modifierFlags]);

	mp_update_mouse_move(p.x, p.y, event.move.deltaX, event.move.deltaY);

	mp_queue_event(&event);
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

	mp_queue_event(&event);
}

- (void)mouseExited:(NSEvent *)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_LEAVE;
	mp_queue_event(&event);
}

- (void)mouseEntered:(NSEvent *)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_MOUSE_ENTER;
	mp_queue_event(&event);
}

- (void)keyDown:(NSEvent*)nsEvent
{
	mp_key_action action = [nsEvent isARepeat] ? MP_KEY_REPEAT : MP_KEY_PRESS;

	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_KEYBOARD_KEY;
	event.key.action = action;
	event.key.code = mp_convert_osx_key([nsEvent keyCode]);
	event.key.mods = mp_convert_osx_mods([nsEvent modifierFlags]);

	str8 label = mp_key_to_label(event.key.code);
	event.key.labelLen = label.len;
	memcpy(event.key.label, label.ptr, label.len);

	mp_update_key_state(&__mpApp.inputState.keyboard.keys[event.key.code], action);

	mp_queue_event(&event);

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

	mp_update_key_state(&__mpApp.inputState.keyboard.keys[event.key.code], MP_KEY_RELEASE);

	mp_queue_event(&event);
}

- (void) flagsChanged:(NSEvent*)nsEvent
{
	mp_event event = {};
	event.window = mp_window_handle_from_ptr(window);
	event.type = MP_EVENT_KEYBOARD_MODS;
	event.key.mods = mp_convert_osx_mods([nsEvent modifierFlags]);

	mp_update_key_mods(event.key.mods);

	mp_queue_event(&event);
}

- (BOOL)performKeyEquivalent:(NSEvent*)nsEvent
{
	if([nsEvent modifierFlags] & NSEventModifierFlagCommand)
	{
		if([nsEvent charactersIgnoringModifiers] == [NSString stringWithUTF8String:"w"])
		{
			[window->osx.nsWindow performClose:self];
			return(YES);
		}
		else if([nsEvent charactersIgnoringModifiers] == [NSString stringWithUTF8String:"q"])
		{
			__mpApp.shouldQuit = true;

			mp_event event = {};
			event.type = MP_EVENT_QUIT;

			mp_queue_event(&event);

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
	NSRect frame = [window->osx.nsView frame];
	return(NSMakeRect(frame.origin.x, frame.origin.y, 0.0, 0.0));
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
	NSString* characters;
	NSEvent* nsEvent = [NSApp currentEvent];
	mp_keymod_flags mods = mp_convert_osx_mods([nsEvent modifierFlags]);

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

			mp_queue_event(&event);
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
	if(__mpApp.displayRefreshCallback)
	{
		__mpApp.displayRefreshCallback(__mpApp.displayRefreshData);
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
	if(!__mpApp.init)
	{
		memset(&__mpApp, 0, sizeof(__mpApp));

		mem_arena_init(&__mpApp.eventArena);

		mp_clock_init();

		LOG_MESSAGE("init keys\n");
		mp_init_osx_keys();
		mp_update_keyboard_layout();
		mp_install_keyboard_layout_listener();

		LOG_MESSAGE("init handles\n");
		mp_init_window_handles();

		LOG_MESSAGE("init event queue\n");
		ringbuffer_init(&__mpApp.eventQueue, 16);

		[MPApplication sharedApplication];
		MPAppDelegate* delegate = [[MPAppDelegate alloc] init];
		[NSApp setDelegate: delegate];

		[NSThread detachNewThreadSelector:@selector(noOpThread:)
		                         toTarget:NSApp
		                         withObject:nil];

		__mpApp.init = true;

		LOG_MESSAGE("run application\n");
		[NSApp run];

		/*
		CGDirectDisplayID displayID = CGMainDisplayID();
		CVDisplayLinkCreateWithCGDisplay(displayID, &__mpApp.displayLink);
		CVDisplayLinkSetOutputCallback(__mpApp.displayLink, DisplayLinkCallback, 0);
		*/

		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		[NSApp activateIgnoringOtherApps:YES];

	}
}}

void mp_terminate()
{
	//TODO: proper app data cleanup (eg delegate, etc)
	if(__mpApp.init)
	{
		mem_arena_release(&__mpApp.eventArena);
		__mpApp = (mp_app){0};
	}
}

bool mp_should_quit()
{
	return(__mpApp.shouldQuit);
}

void mp_do_quit()
{
	__mpApp.shouldQuit = true;
}

void mp_cancel_quit()
{
	__mpApp.shouldQuit = false;
}

void mp_request_quit()
{
	__mpApp.shouldQuit = true;
	mp_event event = {};
	event.type = MP_EVENT_QUIT;
	mp_queue_event(&event);
}

void mp_set_cursor(mp_mouse_cursor cursor)
{
	switch(cursor)
	{
		case MP_MOUSE_CURSOR_ARROW:
		{
			__mpApp.osx.cursor = [NSCursor arrowCursor];
		} break;
		case MP_MOUSE_CURSOR_RESIZE_0:
		{
			__mpApp.osx.cursor = [[NSCursor class] performSelector:@selector(_windowResizeEastWestCursor)];
		} break;
		case MP_MOUSE_CURSOR_RESIZE_90:
		{
			__mpApp.osx.cursor = [[NSCursor class] performSelector:@selector(_windowResizeNorthSouthCursor)];
		} break;
		case MP_MOUSE_CURSOR_RESIZE_45:
		{
			__mpApp.osx.cursor = [[NSCursor class] performSelector:@selector(_windowResizeNorthEastSouthWestCursor)];
		} break;
		case MP_MOUSE_CURSOR_RESIZE_135:
		{
			__mpApp.osx.cursor = [[NSCursor class] performSelector:@selector(_windowResizeNorthWestSouthEastCursor)];
		} break;
		case MP_MOUSE_CURSOR_TEXT:
		{
			__mpApp.osx.cursor = [NSCursor IBeamCursor];
		} break;
	}
	[__mpApp.osx.cursor set];
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

	window->osx.nsWindow = [[MPNativeWindow alloc] initWithMPWindow: window contentRect:rect styleMask:styleMask];
	window->osx.nsWindowDelegate = [[MPNativeWindowDelegate alloc] initWithMPWindow:window];

	[window->osx.nsWindow setDelegate:(id)window->osx.nsWindowDelegate];
	[window->osx.nsWindow setTitle:[NSString stringWithUTF8String:title]];

	if(style & MP_WINDOW_STYLE_NO_TITLE)
	{
		[window->osx.nsWindow setOpaque:NO];
		[window->osx.nsWindow setBackgroundColor:[NSColor clearColor]];
		[window->osx.nsWindow setHasShadow:YES];
	}
	if(style & MP_WINDOW_STYLE_FLOAT)
	{
		[window->osx.nsWindow setLevel:NSFloatingWindowLevel];
		[window->osx.nsWindow setHidesOnDeactivate:YES];
	}
	if(style & MP_WINDOW_STYLE_NO_BUTTONS)
	{
		[[window->osx.nsWindow standardWindowButton:NSWindowCloseButton] setHidden:YES];
		[[window->osx.nsWindow standardWindowButton:NSWindowMiniaturizeButton] setHidden:YES];
		[[window->osx.nsWindow standardWindowButton:NSWindowZoomButton] setHidden:YES];
	}

	MPNativeView* view = [[MPNativeView alloc] initWithMPWindow:window];

	[window->osx.nsWindow setContentView:view];
	[window->osx.nsWindow makeFirstResponder:view];
	[window->osx.nsWindow setAcceptsMouseMovedEvents:YES];

	mp_window_update_rect_cache(window);

	mp_window windowHandle = mp_window_handle_from_ptr(window);

	return(windowHandle);
}//autoreleasepool
}

void mp_window_destroy(mp_window window)
{@autoreleasepool{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		[windowData->osx.nsWindow orderOut:nil];

		[windowData->osx.nsWindow setDelegate:nil];
		[windowData->osx.nsWindowDelegate release];
		windowData->osx.nsWindowDelegate = nil;

		[windowData->osx.nsView release];
		windowData->osx.nsView = nil;

		[windowData->osx.nsWindow close]; //also release the window

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
		[windowData->osx.nsWindow close];
		//NOTE(martin): this will call our window delegate willClose method
	}
}

void* mp_window_native_pointer(mp_window window)
{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		return((__bridge void*)windowData->osx.nsWindow);
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
		[windowData->osx.nsWindow center];
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
		return([windowData->osx.nsWindow isKeyWindow]);
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
		[windowData->osx.nsWindow orderOut:nil];
	}
}}

void mp_window_focus(mp_window window)
{@autoreleasepool{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		[windowData->osx.nsWindow makeKeyWindow];
	}
}}

void mp_window_send_to_back(mp_window window)
{@autoreleasepool{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		[windowData->osx.nsWindow orderBack:nil];
	}
}}

void mp_window_bring_to_front(mp_window window)
{@autoreleasepool{
	mp_window_data* windowData = mp_window_ptr_from_handle(window);
	if(windowData)
	{
		windowData->hidden = false;
		[windowData->osx.nsWindow orderFront:nil];
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

		[windowData->osx.nsWindow setFrame:frame display:YES];

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
		[windowData->osx.nsWindow setFrame:frame display:YES];

		mp_window_update_rect_cache(windowData);
		NSRect contentRect = [[windowData->osx.nsWindow contentView] frame];
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
// platform surface
//--------------------------------------------------------------------

void* mg_osx_surface_native_layer(mg_surface_data* surface)
{
	return((void*)surface->layer.caLayer);
}

vec2 mg_osx_surface_contents_scaling(mg_surface_data* surface)
{@autoreleasepool{
	f32 contentsScale = [surface->layer.caLayer contentsScale];
	vec2 res = {contentsScale, contentsScale};
	return(res);
}}

mp_rect mg_osx_surface_get_frame(mg_surface_data* surface)
{@autoreleasepool{
	CGRect frame = surface->layer.caLayer.frame;
	mp_rect res = {frame.origin.x,
	               frame.origin.y,
	               frame.size.width,
	               frame.size.height};
	return(res);
}}

void mg_osx_surface_set_frame(mg_surface_data* surface, mp_rect frame)
{@autoreleasepool{
	CGRect cgFrame = {{frame.x, frame.y}, {frame.w, frame.h}};
	[surface->layer.caLayer setFrame: cgFrame];
}}

bool mg_osx_surface_get_hidden(mg_surface_data* surface)
{@autoreleasepool{
	return([surface->layer.caLayer isHidden]);
}}

void mg_osx_surface_set_hidden(mg_surface_data* surface, bool hidden)
{@autoreleasepool{
	[CATransaction begin];
	[CATransaction setDisableActions:YES];
	[surface->layer.caLayer setHidden:hidden];
	[CATransaction commit];
}}

void mg_surface_cleanup(mg_surface_data* surface)
{@autoreleasepool{
	[surface->layer.caLayer release];
}}

void mg_surface_init_for_window(mg_surface_data* surface, mp_window_data* window)
{@autoreleasepool{

	surface->nativeLayer = mg_osx_surface_native_layer;
	surface->contentsScaling = mg_osx_surface_contents_scaling;
	surface->getFrame = mg_osx_surface_get_frame;
	surface->setFrame = mg_osx_surface_set_frame;
	surface->getHidden = mg_osx_surface_get_hidden;
	surface->setHidden = mg_osx_surface_set_hidden;

	surface->layer.caLayer = [[CALayer alloc] init];
	[surface->layer.caLayer retain];

	NSRect frame = [[window->osx.nsWindow contentView] frame];
	CGSize size = frame.size;
	surface->layer.caLayer.frame = (CGRect){{0, 0}, size};

	[window->osx.nsView.layer addSublayer: surface->layer.caLayer];
}}

//------------------------------------------------------------------------------------------------
// Remote surfaces
//------------------------------------------------------------------------------------------------
mg_surface_id mg_osx_surface_remote_id(mg_surface_data* surface)
{
	mg_surface_id remoteID = 0;
	if(surface->layer.caContext)
	{
		@autoreleasepool
		{
			remoteID = (mg_surface_id)[surface->layer.caContext contextId];
		}
	}
	return(remoteID);
}

void mg_surface_init_remote(mg_surface_data* surface, u32 width, u32 height)
{@autoreleasepool{

	surface->nativeLayer = mg_osx_surface_native_layer;
	surface->contentsScaling = mg_osx_surface_contents_scaling;
	surface->getFrame = mg_osx_surface_get_frame;
	surface->setFrame = mg_osx_surface_set_frame;
	surface->getHidden = mg_osx_surface_get_hidden;
	surface->setHidden = mg_osx_surface_set_hidden;
	surface->remoteID = mg_osx_surface_remote_id;

	surface->layer.caLayer = [[CALayer alloc] init];
	[surface->layer.caLayer retain];
	[surface->layer.caLayer setFrame: (CGRect){{0, 0}, {width, height}}];

	NSDictionary* dict = [[NSDictionary alloc] init];
	CGSConnectionID connectionID = CGSMainConnectionID();
	surface->layer.caContext = [CAContext contextWithCGSConnection: connectionID options: dict];
	[surface->layer.caContext retain];
	[surface->layer.caContext setLayer:surface->layer.caLayer];
}}

void mg_osx_surface_host_connect(mg_surface_data* surface, mg_surface_id remoteID)
{@autoreleasepool{
	[(CALayerHost*)surface->layer.caLayer setContextId: (CAContextID)remoteID];
}}

void mg_surface_init_host(mg_surface_data* surface, mp_window_data* window)
{@autoreleasepool{

	surface->backend = MG_BACKEND_HOST;
	surface->nativeLayer = mg_osx_surface_native_layer;
	surface->contentsScaling = mg_osx_surface_contents_scaling;
	surface->getFrame = mg_osx_surface_get_frame;
	surface->setFrame = mg_osx_surface_set_frame;
	surface->getHidden = mg_osx_surface_get_hidden;
	surface->setHidden = mg_osx_surface_set_hidden;
	surface->hostConnect = mg_osx_surface_host_connect;

	surface->layer.caLayer = [[CALayerHost alloc] init];
	[surface->layer.caLayer retain];

	NSRect frame = [[window->osx.nsWindow contentView] frame];
	CGSize size = frame.size;
	[surface->layer.caLayer setFrame: (CGRect){{0, 0}, size}];

	[window->osx.nsView.layer addSublayer: surface->layer.caLayer];
}}

mg_surface_data* mg_osx_surface_create_host(mp_window windowHandle)
{
	mg_surface_data* surface = 0;
	mp_window_data* window = mp_window_ptr_from_handle(windowHandle);
	if(window)
	{
		surface = malloc_type(mg_surface_data);
		if(surface)
		{
			mg_surface_init_host(surface, window);
		}
	}
	return(surface);
}

//--------------------------------------------------------------------
// view management
//--------------------------------------------------------------------
/*
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

	[[window->osx.nsWindow contentView] addSubview: view->nsView];

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

	if(!mg_surface_is_nil(view->surface))
	{
		mg_surface_resize(view->surface, frame.w, frame.h);
	}
}
*/
//--------------------------------------------------------------------
// Main loop throttle
//--------------------------------------------------------------------

void mp_set_target_fps(u32 fps)
{
	__mpApp.frameStats.targetFramePeriod = 1./(f64)fps;
	__mpApp.frameStats.workTime = 0;
	__mpApp.frameStats.remainingTime = 0;

	if(__mpApp.osx.frameTimer)
	{
		[__mpApp.osx.frameTimer invalidate];
	}

	__mpApp.osx.frameTimer = [NSTimer timerWithTimeInterval: __mpApp.frameStats.targetFramePeriod
	                          target: [NSApp delegate]
				  selector:@selector(timerElapsed:)
				  userInfo:nil
				  repeats:YES];

	[[NSRunLoop currentRunLoop] addTimer:__mpApp.osx.frameTimer forMode:NSRunLoopCommonModes];
}
/*
void mp_begin_frame()
{
	__mpApp.frameStats.start = mp_get_elapsed_seconds();

	LOG_DEBUG("workTime = %.6f (%.6f fps), remaining = %.6f\n",
	             __mpApp.frameStats.workTime,
	             1/__mpApp.frameStats.workTime,
	             __mpApp.frameStats.remainingTime);

}

void mp_end_frame()
{
	__mpApp.frameStats.workTime = mp_get_elapsed_seconds() - __mpApp.frameStats.start;
	__mpApp.frameStats.remainingTime = __mpApp.frameStats.targetFramePeriod - __mpApp.frameStats.workTime;

	while(__mpApp.frameStats.remainingTime > 100e-9)
	{
		if(__mpApp.frameStats.remainingTime > 10e-6)
		{
			mp_sleep_nanoseconds(__mpApp.frameStats.remainingTime*0.8*1e9);
		}
		__mpApp.frameStats.workTime = mp_get_elapsed_seconds() - __mpApp.frameStats.start;
		__mpApp.frameStats.remainingTime = __mpApp.frameStats.targetFramePeriod - __mpApp.frameStats.workTime;
	}
}
*/

//--------------------------------------------------------------------
// Events handling
//--------------------------------------------------------------------

void mp_set_live_resize_callback(mp_live_resize_callback callback, void* data)
{
	__mpApp.liveResizeCallback = callback;
	__mpApp.liveResizeData = data;
}


void mp_pump_events(f64 timeout)
{
	__mpApp.inputState.frameCounter++;

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

/*
void mp_run_loop()
{@autoreleasepool {

	//CVDisplayLinkStart(__mpApp.displayLink);

	while(!__mpApp.shouldQuit)
	{
		mp_event event;
		while(mp_next_event(&event))
		{
			//send pending event that might have accumulated before we started run loop
			if(__mpApp.eventCallback)
			{
				__mpApp.eventCallback(event, __mpApp.eventData);
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

	//CVDisplayLinkStop(__mpApp.displayLink);
}}
*/
void mp_end_input_frame()
{
	//////////////////////////////////////////////////////////////////////////////////////////////
	//TODO: make sure we call arena clear once per event frame, even when using runloop etc...
	//////////////////////////////////////////////////////////////////////////////////////////////
	__mpApp.inputState.frameCounter++;
	mem_arena_clear(&__mpApp.eventArena);
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
	str8_list_push(scratch, &list, STR8("/"));
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
