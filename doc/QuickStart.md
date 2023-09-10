--------
**DISCLAIMER: This project is very much a Work In Progress. We're making it accessible in this very early state so that participants to the [Wheel Reinvention Jam 2023](https://handmade.network/jam/2023) can try it out and maybe use it as their jamming platform. Expect bugs, missing and/or incomplete features, unstable APIs, and sparse documentation. Some current issues might be a show stopper for you, so make sure you can build and run the sample apps before jumping in.**

**If you do choose to try out Orca anyway, well thanks! We'll do our best to answer your questions, and we'd really appreciate to hear your feedback!**

--------

# Orca Quick Start Guide
---

This is a short introduction to developping an application that can be run by the Orca runtime. We'll present the basic structure of an Orca application, and walk through a simple example in C.

## Basic structure

Orca exposes a number of types and functions to applications. In order to use them the first thing to do is to include `orca.h`.

```
#include<orca.h>
```

The Orca runtime manages the application's window and event loop. In order to receive a specific kind of event, you can define an associated _event handler_ and export it to the runtime. For instance, to be notified when your application's window is resized, you should define the `oc_on_resize()` handler:

```
ORCA_EXPORT void oc_on_resize(u32 width, u32 height)
{
	// handle the window resize event
}
```

The `ORCA_EXPORT` macro makes the handler visible to the orca runtime, which automatically binds it to the window resize event.

Handlers are optional. If you don't care about an event, you can just omit the associated handler. You probably want to define at least two important handlers:

- `oc_on_init()` is called once when your application starts and can be use to initialize your application's resources
- `oc_on_frame_refresh()` is called when your application needs to render a new frame, typically tied to the refresh rate of the monitor.

For a list of available handlers and their signatures, see the [app cheatsheet](../doc/cheatsheets/cheatsheet_app.h).


## Clock Example

Let's look at the [clock example](../samples/clock). This is a simple app that shows an analog clock and showcases a couple of interesting Orca APIs.

Open [`main.c`](../samples/clock/src/main.c) and look at the definition of `oc_on_init()`. This handler is called when the applications starts, right after the application window has been created.

The first thing we do here is set the title and dimensions of the window. We then create the graphics resources that we'll use to draw the clock onto the window.

### Graphics surfaces

The next line of `oc_init()` creates a _graphics surface_. A surface represents a destination you can draw into using a specific API. In this sample, we're going to use a canvas surface, which allows drawing with a 2D vector graphics API. Other samples use a GLES surface to draw with the OpenGL ES API.

Before drawing into it, the surface must be selected as the current surface by calling `oc_surface_select()`. Once all drawing is done you can display the result by calling `oc_surface_present()`.

### Canvas

After creating the surface, we create a _canvas_. A canvas holds some context for drawing commands, like the current color or stroke width, as well as a command buffer that records all drawing commands. All canvas drawing functions use an implicit _current canvas_. You can select a canvas to be the current canvas by calling `oc_canvas_select()`, as seen at the begining of `oc_on_frame_refresh()`.

Canvas drawing functions like `oc_fill()` or `oc_stroke` merely add to the current canvas command buffer. You can later render those commands onto a canvas surface by calling `oc_render()`.

To summarize, the general structure of canvas drawing code is like the following:

```
oc_canvas_select(canvas); // make the canvas current

//... add commands to the canvas command buffer using drawing functions

oc_surface_select(surface);  // select the canvas surface
oc_render(canvas);           // render the canvas commands into it
oc_surface_present(surface); // display the result
```

### Drawing

Canvas drawing functions can be roughly divided into three groups:

- Path functions like `oc_line_to()` or `oc_cubic_to()` are used to specify paths using lines and curves.
- Attributes setup functions like `oc_set_color()` or `oc_set_width()` are used to set attributes used by subsequent commands.
- Command functions like `oc_stroke()` and `oc_fill()` encode commands into the canvas command buffer using the current path and attributes.

Some helpers combine a path specification and a command, like `oc_circle_fill()`.

As an example, the back of the clock is drawn using these two calls:

```
	// clock backing
    oc_set_color_rgba(1, 1, 1, 1);
    oc_circle_fill(centerX, centerY, clockRadius);
```

For a list of canvas drawing functions, see the [graphics API cheatsheet](../doc/cheatsheets/cheatsheet_graphics.h).

#### Transforms

A special case of attribute setting function is the pair `oc_matrix_push()` and `oc_matrix_pop()`, which are used to manipulate a stack of transform matrices:

- `oc_matrix_push()` multiplies the matrix currently on top of the stack with its argument, and pushes the result on the stack.
- `oc_matrix_pop()` pops a matrix from the stack.

The matrix on the top of the stack at the time a command is encoded is used to transform the path of that command.

You can see an example of using transform matrices when drawing the clock's hands:

```
    // hours hand
    oc_matrix_push(mat_transform(centerX, centerY, hoursRotation));
    {
        oc_set_color_rgba(.2, 0.2, 0.2, 1);
        oc_rounded_rectangle_fill(0, -7.5 * uiScale, clockRadius * 0.5f, 15 * uiScale, 5 * uiScale);
    }
    oc_matrix_pop();
```

### Fonts and text

Going back to `oc_init()`, after creating a surface and a canvas, we create a font that we will use to draw the numbers on the clock's face:

```
    oc_unicode_range ranges[5] = {
        OC_UNICODE_BASIC_LATIN,
        OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
        OC_UNICODE_LATIN_EXTENDED_A,
        OC_UNICODE_LATIN_EXTENDED_B,
        OC_UNICODE_SPECIALS
    };

    font = oc_font_create_from_path(OC_STR8("/segoeui.ttf"), 5, ranges);
```

The font is loaded from a font file located in a data folder inside the app bundle. By default, Orca apps use this data folder as their "root" for file operations.

Along with the path of the font file, we pass to the creation function the unicode ranges we want to load.

We then use the font to draw the clock's face:

```
    // clock face
    for(int i = 0; i < oc_array_size(clockNumberStrings); ++i)
    {
        oc_rect textRect = oc_text_bounding_box(font, fontSize, clockNumberStrings[i]);
        textRect.h -= 10 * uiScale; // oc_text_bounding_box height doesn't seem to be a tight fit around the glyph

        const f32 angle = i * ((M_PI * 2) / 12.0f) - (M_PI / 2);
        oc_mat2x3 transform = mat_transform(centerX - (textRect.w / 2), centerY + (textRect.h / 2), angle);
        oc_vec2 pos = oc_mat2x3_mul(transform, (oc_vec2){ clockRadius * 0.8f, 0 });

        oc_set_color_rgba(0.2, 0.2, 0.2, 1);
        oc_text_fill(pos.x, pos.y, clockNumberStrings[i]);
    }
```

### Logging and asserts

The runtime has a console overlay whose visiblity can be toggled on and off with `Shift + Cmd + D` on macOS, or `Shift + Ctrl + D` on Windows. Your application can log messages, warnings or errors to that console using the following functions:

```
void oc_log_info(const char* fmt, ...);    // informational messages
void oc_log_warning(const char* fmt, ...); // warnings, displayed in orange.
void oc_log_error(const char* fmt, ...);  // errors, displayed in red.
```

If you started the application from a terminal, the log entries are also duplicated there.

You can assert on a condition using `OC_ASSERT(test, fmt, ...)`. If the test fails, the runtime displays a message box including your message, and terminates the application.

You can unconditionally abort the application with a message box using `OC_ABORT(fmt, ...)`.

## Where to go next?

For more examples of how to use Orca APIs, you can look at the other [sample apps](../samples):

- [breakout](./samples/breakout) is a mini breakout game making use of the vector graphics API. It demonstrates using input and drawing images.
- 	[triangle](./samples/triangle) shows how to draw a spining triangle using the GLES API.
-  [fluid](./samples/fluid) is a fluid simulation using a more complex GLES setup.
-  [ui](./samples/ui) showcases the UI API and Orca's default UI widgets.

For a list of Orca APIs, you can look at the [API cheatsheets](../doc/cheatsheets).

You can also ask questions in the [Handmade Network Discord](https://discord.gg/hmn), in particular in the [#orca](https://discord.com/channels/239737791225790464/1121811864066732082) channel.
