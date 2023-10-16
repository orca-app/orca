const std = @import("std");
const oc = @import("orca");
const ui = oc.ui;

// Register event handlers with orca
comptime {
    oc.registerHandler(onInit, .Init);
    oc.registerHandler(onResize, .Resize);
    oc.registerHandler(onRawEvent, .RawEvent);
    oc.registerHandler(onFrameRefresh, .FrameRefresh);
}

var frame_size: oc.Vec2 = .{ .x = 1200, .y = 838 };

var surface: oc.Surface = undefined;
var canvas: oc.Canvas = undefined;
var font_regular: oc.Font = undefined;
var font_bold: oc.Font = undefined;
var ui_ctx: ui.Context = undefined;
var text_arena: oc.Arena = undefined;
var log_arena: oc.Arena = undefined;
var log_lines: oc.Str8List = undefined;

const Cmd = enum {
    None,
    SetDarkTheme,
    SetLightTheme,
};
var cmd: Cmd = .None;

fn onInit() !void {
    oc.windowSetTitle("Orca Zig UI Demo");
    oc.windowSetSize(frame_size);

    surface = oc.Surface.canvas();
    canvas = oc.Canvas.create();
    ui.init(&ui_ctx);

    var fonts = [_]*oc.Font{ &font_regular, &font_bold };
    var font_names = [_][]const u8{ "/OpenSans-Regular.ttf", "/OpenSans-Bold.ttf" };
    for (fonts, font_names) |font, name| {
        var scratch = oc.Arena.scratchBegin();
        defer scratch.end();

        var file = oc.File.open(name, .{ .read = true }, .{}) catch |e| {
            oc.log.err("Couldn't open file {s}", .{name}, @src());
            return e;
        };

        var size = try file.getSize();
        var buffer = try scratch.arena.push(@intCast(size));
        _ = try file.read(buffer);
        file.close();

        var ranges = oc.UnicodeRange.range(&[_]oc.UnicodeRange.Enum{
            .BasicLatin,
            .C1ControlsAndLatin1Supplement,
            .LatinExtendedA,
            .LatinExtendedB,
            .Specials,
        });

        font.* = oc.Font.createFromMemory(buffer, &ranges);
    }

    text_arena = oc.Arena.init();
    log_arena = oc.Arena.init();
    log_lines = oc.Str8List.init();
}

fn onRawEvent(event: *const oc.Event) void {
    oc.ui.processCEvent(event.c_event);
}

fn onResize(width: u32, height: u32) void {
    frame_size.x = @floatFromInt(width);
    frame_size.y = @floatFromInt(height);
}

fn onFrameRefresh() void {
    var scratch = oc.Arena.scratchBegin();
    defer scratch.end();

    switch (cmd) {
        .SetDarkTheme => ui.setTheme(ui.dark_theme),
        .SetLightTheme => ui.setTheme(ui.light_theme),
        .None => {},
    }
    cmd = .None;

    var default_style = ui.Style{ .font = font_regular };
    {
        ui.beginFrame(frame_size, &default_style);
        defer ui.endFrame();

        //--------------------------------------------------------------------------------------------
        // Menu bar
        //--------------------------------------------------------------------------------------------
        {
            ui.menuBarBegin("menu_bar");
            defer ui.menuBarEnd();

            {
                ui.menuBegin("File");
                defer ui.menuEnd();

                if (ui.menuButton("Quit").pressed) {
                    oc.requestQuit();
                }
            }

            {
                ui.menuBegin("Theme");
                defer ui.menuEnd();

                if (ui.menuButton("Dark theme").pressed) {
                    cmd = .SetDarkTheme;
                }
                if (ui.menuButton("Light theme").pressed) {
                    cmd = .SetLightTheme;
                }
            }
        }

        {
            ui.panelBegin("main panel", .{});
            defer ui.panelEnd();

            {
                ui.styleNext(.{
                    .size = .{
                        .width = .fill_parent,
                        .height = .{ .custom = .{ .kind = .Parent, .value = 1, .relax = 1 } },
                    },
                    .layout = .{
                        .axis = .X,
                        .margin = .{ .x = 16, .y = 16 },
                        .spacing = 16,
                    },
                });
                _ = ui.boxBegin("Background", .{ .draw_background = true });
                defer _ = ui.boxEnd();

                widgets(scratch.arena);

                styling(scratch.arena);
            }
        }
    }

    _ = canvas.select();
    surface.select();

    oc.Canvas.setColor(ui_ctx.theme.bg0);
    oc.Canvas.clear();

    ui.draw();
    canvas.render();
    surface.present();
}

var checkbox_checked: bool = false;
var v_slider_value: f32 = 0;
var v_slider_logged_value: f32 = 0;
var v_slider_log_time: f64 = 0;
var radio_selected: usize = 0;
var h_slider_value: f32 = 0;
var h_slider_logged_value: f32 = 0;
var h_slider_log_time: f64 = 0;
var text: []const u8 = "Text box";
var selected: ?usize = null;

fn widgets(arena: *oc.Arena) void {
    columnBegin("Widgets", 1.0 / 3.0);
    defer columnEnd();

    {
        ui.styleNext(.{
            .size = .{
                .width = .fill_parent,
            },
            .layout = .{
                .axis = .X,
                .spacing = 32,
            },
        });
        _ = ui.boxBegin("top", .{});
        defer _ = ui.boxEnd();

        {
            ui.styleNext(.{
                .layout = .{
                    .axis = .Y,
                    .spacing = 24,
                },
            });
            _ = ui.boxBegin("top_left", .{});
            defer _ = ui.boxEnd();

            //-----------------------------------------------------------------------------
            // Label
            //-----------------------------------------------------------------------------
            _ = ui.makeLabel("Label");

            //-----------------------------------------------------------------------------
            // Button
            //-----------------------------------------------------------------------------
            if (ui.button("Button").clicked) {
                logPush("Button clicked");
            }

            {
                ui.styleNext(.{
                    .layout = .{
                        .axis = .X,
                        .alignment = .{ .y = .Center },
                        .spacing = 8,
                    },
                });
                _ = ui.boxBegin("checkbox", .{});
                defer _ = ui.boxEnd();

                //-------------------------------------------------------------------------
                // Checkbox
                //-------------------------------------------------------------------------
                if (ui.checkbox("checkbox", &checkbox_checked).clicked) {
                    if (checkbox_checked) {
                        logPush("Checkbox checked");
                    } else {
                        logPush("Checkbox unhecked");
                    }
                }

                _ = ui.makeLabel("Checkbox");
            }
        }

        //---------------------------------------------------------------------------------
        // Vertical slider
        //---------------------------------------------------------------------------------
        ui.styleNext(.{ .size = .{ .height = .{ .pixels = 130 } } });
        _ = ui.slider("v_slider", &v_slider_value);

        var now = oc.clock.time(.Monotonic);
        if ((now - v_slider_log_time) >= 0.2 and v_slider_value != v_slider_logged_value) {
            logPushf("Vertical slider moved to {d:.3}", .{v_slider_value});
            v_slider_logged_value = v_slider_value;
            v_slider_log_time = now;
        }

        {
            ui.styleNext(.{
                .layout = .{
                    .axis = .Y,
                    .spacing = 24,
                },
            });
            _ = ui.boxBegin("top right", .{});
            defer _ = ui.boxEnd();

            //-----------------------------------------------------------------------------
            // Tooltip
            //-----------------------------------------------------------------------------
            if (ui.makeLabel("Tooltip").hovering) {
                ui.tooltip("Hi");
            }

            //-----------------------------------------------------------------------------
            // Radio group
            //-----------------------------------------------------------------------------
            var options = [_][]const u8{
                "Radio 1",
                "Radio 2",
            };
            var radio_group_info = ui.RadioGroupInfo{
                .selected_index = radio_selected,
                .options = &options,
            };
            var result = ui.radioGroup("radio_group", &radio_group_info);
            radio_selected = result.selected_index.?;
            if (result.changed) {
                logPushf("Selected {s}", .{options[radio_selected]});
            }

            //-----------------------------------------------------------------------------
            // Horizontal slider
            //-----------------------------------------------------------------------------
            ui.styleNext(.{ .size = .{ .width = .{ .pixels = 130 } } });
            _ = ui.slider("h_slider", &h_slider_value);

            if ((now - h_slider_log_time) >= 0.2 and h_slider_value != h_slider_logged_value) {
                logPushf("Slider moved to {d:.3}", .{h_slider_value});
                h_slider_logged_value = h_slider_value;
                h_slider_log_time = now;
            }
        }
    }

    //-------------------------------------------------------------------------------------
    // Text box
    //-------------------------------------------------------------------------------------
    ui.styleNext(.{
        .size = .{
            .width = .{ .pixels = 305 },
            .height = .text,
        },
    });
    var textResult = ui.textBox("text", arena, text);
    if (textResult.changed) {
        text_arena.clear();
        text = text_arena.pushStr(textResult.text) catch {
            oc.log.err("Out of memory", .{}, @src());
            oc.requestQuit();
            return;
        };
    }
    if (textResult.accepted) {
        logPushf("Entered text {s}", .{text});
    }

    //-------------------------------------------------------------------------------------
    // Select
    //-------------------------------------------------------------------------------------
    var options = [_][]const u8{
        "Option 1",
        "Option 2",
    };
    var select_popup_info = ui.SelectPopupInfo{
        .selected_index = selected,
        .options = &options,
        .placeholder = "Select",
    };
    var selectResult = ui.selectPopup("select", &select_popup_info);
    if (selectResult.selected_index != selected) {
        logPushf("Selected {s}", .{options[selectResult.selected_index.?]});
    }
    selected = selectResult.selected_index;

    //-------------------------------------------------------------------------------------
    // Scrollable panel
    //-------------------------------------------------------------------------------------
    {
        ui.styleNext(.{
            .size = .{
                .width = .fill_parent,
                .height = .{
                    .custom = .{ .kind = .Parent, .value = 1, .relax = 1, .min_size = 200 },
                },
            },
            .bg_color = ui_ctx.theme.bg2,
            .border_color = ui_ctx.theme.border,
            .border_size = 1,
            .roundness = ui_ctx.theme.roundness_small,
        });
        _ = ui.panelBegin("log", .{ .draw_background = true, .draw_border = true });
        defer ui.panelEnd();

        {
            ui.styleNext(.{
                .layout = .{
                    .margin = .{ .x = 16, .y = 16 },
                },
            });
            _ = ui.boxBegin("contents", .{});
            defer _ = ui.boxEnd();

            if (log_lines.list.empty()) {
                ui.styleNext(.{ .color = ui_ctx.theme.text2 });
                _ = ui.makeLabel("Log");
            }

            var i: i32 = 0;
            var log_lines_iter = log_lines.iter();
            while (log_lines_iter.next()) |log_line| {
                var buf: [15]u8 = undefined;
                var id = std.fmt.bufPrint(&buf, "{d}", .{i}) catch unreachable;
                _ = ui.boxBegin(id, .{});
                defer _ = ui.boxEnd();

                _ = ui.makeLabel(log_line.string.slice());

                i += 1;
            }
        }
    }
}

var styling_selected_radio: ?usize = 0;
var unselected_width: f32 = 16;
var unselected_height: f32 = 16;
var unselected_roundness: f32 = 8;
var unselected_bg_color: oc.Color = oc.Color.rgba(0.086, 0.086, 0.102, 1);
var unselected_border_color: oc.Color = oc.Color.rgba(0.976, 0.976, 0.976, 0.35);
var unselected_border_size: f32 = 1;
var unselected_when_status: ui.Status = .{};
var unselected_status_index: ?usize = 0;
var selected_width: f32 = 16;
var selected_height: f32 = 16;
var selected_roundness: f32 = 8;
var selected_center_color: oc.Color = oc.Color.rgba(1, 1, 1, 1);
var selected_bg_color: oc.Color = oc.Color.rgba(0.33, 0.66, 1, 1);
var selected_when_status: ui.Status = .{};
var selected_status_index: ?usize = 0;
var label_font_color: oc.Color = oc.Color.rgba(0.976, 0.976, 0.976, 1);
var label_font_color_selected: ?usize = 0;
var label_font: *oc.Font = &font_regular;
var label_font_selected: ?usize = 0;
var label_font_size: f32 = 14;

fn styling(arena: *oc.Arena) void {
    //-----------------------------------------------------------------------------------------
    // Styling
    //-----------------------------------------------------------------------------------------
    // Initial values here are hardcoded from the dark theme and everything is overridden all
    // the time. In a real program you'd only override what you need and supply the values from
    // ui_ctx.theme or ui_ctx.theme.palette.
    //
    // Rule-based styling is described at
    // https://www.forkingpaths.dev/posts/23-03-10/rule_based_styling_imgui_ctx.html
    columnBegin("Styling", 2.0 / 3.0);
    defer columnEnd();

    {
        ui.styleNext(.{
            .size = .{
                .width = .fill_parent,
                .height = .{ .pixels = 152 },
            },
            .layout = .{
                .margin = .{ .x = 310, .y = 16 },
            },
            .bg_color = ui.dark_theme.bg0,
            .roundness = ui.dark_theme.roundness_small,
        });
        _ = ui.boxBegin("styled_radios", .{ .draw_background = true, .draw_border = true });
        defer _ = ui.boxEnd();

        resetNextRadioGroupToDarkTheme(arena);

        var unselected_tag = ui.Tag.make("radio");
        var unselected_pattern = ui.Pattern.init();
        unselected_pattern.push(arena, .{ .sel = .{ .tag = unselected_tag } });
        if (!unselected_when_status.empty()) {
            unselected_pattern.push(arena, .{ .op = .And, .sel = .{ .status = unselected_when_status } });
        }
        ui.styleMatchAfter(unselected_pattern, .{
            .size = .{
                .width = .{ .pixels = unselected_width },
                .height = .{ .pixels = unselected_height },
            },
            .bg_color = unselected_bg_color,
            .border_color = unselected_border_color,
            .border_size = unselected_border_size,
            .roundness = unselected_roundness,
        });

        var selected_tag = ui.Tag.make("radio_selected");
        var selected_pattern = ui.Pattern.init();
        selected_pattern.push(arena, .{ .sel = .{ .tag = selected_tag } });
        if (!selected_when_status.empty()) {
            selected_pattern.push(arena, .{ .op = .And, .sel = .{ .status = selected_when_status } });
        }
        ui.styleMatchAfter(selected_pattern, .{
            .size = .{
                .width = .{ .pixels = selected_width },
                .height = .{ .pixels = selected_height },
            },
            .color = selected_center_color,
            .bg_color = selected_bg_color,
            .roundness = selected_roundness,
        });

        var label_tag = ui.Tag.make("label");
        var label_pattern = ui.Pattern.init();
        label_pattern.push(arena, .{ .sel = .{ .tag = label_tag } });
        ui.styleMatchAfter(label_pattern, .{
            .color = label_font_color,
            .font = label_font.*,
            .font_size = label_font_size,
        });

        var options = [_][]const u8{
            "I",
            "Am",
            "Stylish",
        };
        var radio_group_info = ui.RadioGroupInfo{
            .selected_index = styling_selected_radio,
            .options = &options,
        };
        var result = ui.radioGroup("radio_group", &radio_group_info);
        styling_selected_radio = result.selected_index;
    }

    {
        ui.styleNext(.{ .layout = .{ .axis = .X, .spacing = 32 } });
        _ = ui.boxBegin("controls", .{});
        defer _ = ui.boxEnd();

        {
            ui.styleNext(.{ .layout = .{ .axis = .Y, .spacing = 16 } });
            _ = ui.boxBegin("unselected", .{});
            defer _ = ui.boxEnd();

            ui.styleNext(.{ .font_size = 16 });
            _ = ui.makeLabel("Radio style");

            {
                ui.styleNext(.{ .layout = .{ .spacing = 4 } });
                _ = ui.boxBegin("size", .{});
                defer _ = ui.boxEnd();

                var width_slider = (unselected_width - 8) / 16;
                labeledSlider("Width", &width_slider);
                unselected_width = 8 + width_slider * 16;

                var height_slider = (unselected_height - 8) / 16;
                labeledSlider("Height", &height_slider);
                unselected_height = 8 + height_slider * 16;

                var roundness_slider = (unselected_roundness - 4) / 8;
                labeledSlider("Roundness", &roundness_slider);
                unselected_roundness = 4 + roundness_slider * 8;
            }

            {
                ui.styleNext(.{ .layout = .{ .spacing = 4 } });
                _ = ui.boxBegin("background", .{});
                defer _ = ui.boxEnd();

                labeledSlider("Background R", &unselected_bg_color.r);
                labeledSlider("Background G", &unselected_bg_color.g);
                labeledSlider("Background B", &unselected_bg_color.b);
                labeledSlider("Background A", &unselected_bg_color.a);
            }

            {
                ui.styleNext(.{ .layout = .{ .spacing = 4 } });
                _ = ui.boxBegin("border", .{});
                defer _ = ui.boxEnd();

                labeledSlider("Border R", &unselected_border_color.r);
                labeledSlider("Border G", &unselected_border_color.g);
                labeledSlider("Border B", &unselected_border_color.b);
                labeledSlider("Border A", &unselected_border_color.a);
            }

            var border_size_slider = unselected_border_size / 5;
            labeledSlider("Border size", &border_size_slider);
            unselected_border_size = border_size_slider * 5;

            {
                ui.styleNext(.{ .layout = .{ .spacing = 10 } });
                _ = ui.boxBegin("status_override", .{});
                defer _ = ui.boxEnd();

                _ = ui.makeLabel("Override");

                var status_options = [_][]const u8{
                    "Always",
                    "When hovering",
                    "When active",
                };
                var status_info = ui.RadioGroupInfo{
                    .selected_index = unselected_status_index,
                    .options = &status_options,
                };
                var status_result = ui.radioGroup("status", &status_info);
                unselected_status_index = status_result.selected_index;
                unselected_when_status = switch (unselected_status_index.?) {
                    0 => .{},
                    1 => .{ .hover = true },
                    2 => .{ .active = true },
                    else => unreachable,
                };
            }
        }

        {
            ui.styleNext(.{ .layout = .{ .axis = .Y, .spacing = 16 } });
            _ = ui.boxBegin("selected", .{});
            defer _ = ui.boxEnd();

            ui.styleNext(.{ .font_size = 16 });
            _ = ui.makeLabel("Radio selected style");

            {
                ui.styleNext(.{ .layout = .{ .spacing = 4 } });
                _ = ui.boxBegin("size", .{});
                defer _ = ui.boxEnd();

                var width_slider = (selected_width - 8) / 16;
                labeledSlider("Width", &width_slider);
                selected_width = 8 + width_slider * 16;

                var height_slider = (selected_height - 8) / 16;
                labeledSlider("Height", &height_slider);
                selected_height = 8 + height_slider * 16;

                var roundness_slider = (selected_roundness - 4) / 8;
                labeledSlider("Roundness", &roundness_slider);
                selected_roundness = 4 + roundness_slider * 8;
            }

            {
                ui.styleNext(.{ .layout = .{ .spacing = 4 } });
                _ = ui.boxBegin("color", .{});
                defer _ = ui.boxEnd();

                labeledSlider("Center R", &selected_center_color.r);
                labeledSlider("Center G", &selected_center_color.g);
                labeledSlider("Center B", &selected_center_color.b);
                labeledSlider("Center A", &selected_center_color.a);
            }

            {
                ui.styleNext(.{ .layout = .{ .spacing = 4 } });
                _ = ui.boxBegin("background", .{});
                defer _ = ui.boxEnd();

                labeledSlider("Background R", &selected_bg_color.r);
                labeledSlider("Background G", &selected_bg_color.g);
                labeledSlider("Background B", &selected_bg_color.b);
                labeledSlider("Background A", &selected_bg_color.a);
            }

            {
                ui.styleNext(.{ .layout = .{ .spacing = 10 } });
                _ = ui.boxBegin("status_override", .{});
                defer _ = ui.boxEnd();

                ui.styleNext(.{ .size = .{ .height = .{ .pixels = 30 } } });
                _ = ui.boxMake("spacer", .{});

                _ = ui.makeLabel("Override");

                var status_options = [_][]const u8{
                    "Always",
                    "When hovering",
                    "When active",
                };
                var status_info = ui.RadioGroupInfo{
                    .selected_index = selected_status_index,
                    .options = &status_options,
                };
                var status_result = ui.radioGroup("status", &status_info);
                selected_status_index = status_result.selected_index;
                selected_when_status = switch (selected_status_index.?) {
                    0 => .{},
                    1 => .{ .hover = true },
                    2 => .{ .active = true },
                    else => unreachable,
                };
            }
        }

        {
            ui.styleNext(.{ .layout = .{ .axis = .Y, .spacing = 10 } });
            _ = ui.boxBegin("label", .{});
            defer _ = ui.boxEnd();

            ui.styleNext(.{ .font_size = 16 });
            _ = ui.makeLabel("Label style");

            {
                ui.styleNext(.{ .layout = .{ .axis = .X, .spacing = 8 } });
                _ = ui.boxBegin("font_color", .{});
                defer _ = ui.boxEnd();

                ui.styleMatchAfter(ui.Pattern.owner(), .{
                    .size = .{ .width = .{ .pixels = 100 } },
                });
                _ = ui.makeLabel("Font color");

                var color_names = [_][]const u8{
                    "Default",
                    "Red",
                    "Orange",
                    "Amber",
                    "Yellow",
                    "Lime",
                    "Light green",
                    "Green",
                };
                var colors = [_]oc.Color{
                    ui.dark_theme.text0,
                    ui.dark_theme.palette.red5,
                    ui.dark_theme.palette.orange5,
                    ui.dark_theme.palette.amber5,
                    ui.dark_theme.palette.yellow5,
                    ui.dark_theme.palette.lime5,
                    ui.dark_theme.palette.light_green5,
                    ui.dark_theme.palette.green5,
                };
                var color_info = ui.SelectPopupInfo{
                    .selected_index = label_font_color_selected,
                    .options = &color_names,
                };
                var color_result = ui.selectPopup("color", &color_info);
                label_font_color_selected = color_result.selected_index;
                label_font_color = colors[label_font_color_selected.?];
            }

            {
                ui.styleNext(.{ .layout = .{ .axis = .X, .spacing = 8 } });
                _ = ui.boxBegin("font", .{});
                defer _ = ui.boxEnd();

                ui.styleMatchAfter(ui.Pattern.owner(), .{
                    .size = .{ .width = .{ .pixels = 100 } },
                });
                _ = ui.makeLabel("Font");

                var font_names = [_][]const u8{
                    "Regular",
                    "Bold",
                };
                var fonts = [_]*oc.Font{
                    &font_regular,
                    &font_bold,
                };
                var font_info = ui.SelectPopupInfo{
                    .selected_index = label_font_selected,
                    .options = &font_names,
                };
                var font_result = ui.selectPopup("font_style", &font_info);
                label_font_selected = font_result.selected_index;
                label_font = fonts[label_font_selected.?];
            }

            var font_size_slider = (label_font_size - 8) / 16;
            labeledSlider("Font size", &font_size_slider);
            label_font_size = 8 + font_size_slider * 16;
        }
    }
}

fn columnBegin(header: []const u8, widthFraction: f32) void {
    ui.styleNext(.{
        .size = .{
            .width = .{
                .custom = .{ .kind = .Parent, .value = widthFraction, .relax = 1 },
            },
            .height = .fill_parent,
        },
        .layout = .{
            .axis = .Y,
            .margin = .{ .y = 8 },
            .spacing = 24,
        },
        .bg_color = ui_ctx.theme.bg1,
        .border_color = ui_ctx.theme.border,
        .border_size = 1,
        .roundness = ui_ctx.theme.roundness_small,
    });
    _ = ui.boxBegin(header, .{ .draw_background = true, .draw_border = true });

    {
        ui.styleNext(.{
            .size = .{ .width = .fill_parent },
            .layout = .{ .alignment = .{ .x = .Center } },
        });
        _ = ui.boxBegin("header", .{});
        defer _ = ui.boxEnd();

        ui.styleNext(.{ .font_size = 18 });
        _ = ui.makeLabel(header);
    }

    ui.styleNext(.{
        .size = .{
            .width = .fill_parent,
            .height = .{
                .custom = .{ .kind = .Parent, .value = 1, .relax = 1 },
            },
        },
        .layout = .{
            .alignment = .{ .x = .Start },
            .margin = .{ .x = 16 },
            .spacing = 24,
        },
    });
    _ = ui.boxBegin("contents", .{});
}

fn columnEnd() void {
    _ = ui.boxEnd(); // contents
    _ = ui.boxEnd(); // column
}

fn labeledSlider(label: []const u8, value: *f32) void {
    ui.styleNext(.{ .layout = .{ .axis = .X, .spacing = 8 } });
    _ = ui.boxBegin(label, .{});
    defer _ = ui.boxEnd();

    ui.styleMatchAfter(ui.Pattern.owner(), .{
        .size = .{ .width = .{ .pixels = 100 } },
    });
    _ = ui.makeLabel(label);

    ui.styleNext(.{
        .size = .{ .width = .{ .pixels = 100 } },
    });
    _ = ui.slider("slider", value);
}

fn logPush(line: []const u8) void {
    log_lines.push(&log_arena, oc.Str8.fromSlice(line)) catch {
        oc.log.err("Out of memory", .{}, @src());
        oc.requestQuit();
        return;
    };
}

fn logPushf(comptime fmt: []const u8, args: anytype) void {
    var str = oc.Str8.pushf(&log_arena, fmt, args) catch {
        oc.log.err("Out of memory", .{}, @src());
        oc.requestQuit();
        return;
    };
    log_lines.push(&log_arena, str) catch {
        oc.log.err("Out of memory", .{}, @src());
        oc.requestQuit();
        return;
    };
}

/// This makes sure the light theme doesn't break the styling overrides
/// You won't need it in a real program as long as your colors come from ui_ctx.theme or ui_ctx.theme.palette
fn resetNextRadioGroupToDarkTheme(arena: *oc.Arena) void {
    var unselected_tag = ui.Tag.make("radio");
    var unselected_pattern = ui.Pattern.init();
    unselected_pattern.push(arena, .{ .sel = .{ .tag = unselected_tag } });
    ui.styleMatchAfter(unselected_pattern, .{
        .border_color = ui.dark_theme.text3,
        .border_size = 1,
    });

    var unselected_hover_pattern = ui.Pattern.init();
    unselected_hover_pattern.push(arena, .{ .sel = .{ .tag = unselected_tag } });
    unselected_hover_pattern.push(arena, .{ .op = .And, .sel = .{ .status = .{ .hover = true } } });
    ui.styleMatchAfter(unselected_hover_pattern, .{
        .bg_color = ui.dark_theme.fill0,
        .border_color = ui.dark_theme.primary,
    });

    var unselected_active_pattern = ui.Pattern.init();
    unselected_active_pattern.push(arena, .{ .sel = .{ .tag = unselected_tag } });
    unselected_active_pattern.push(arena, .{ .op = .And, .sel = .{ .status = .{ .active = true } } });
    ui.styleMatchAfter(unselected_active_pattern, .{
        .bg_color = ui.dark_theme.fill1,
        .border_color = ui.dark_theme.primary,
    });

    var selected_tag = ui.Tag.make("radio_selected");
    var selected_pattern = ui.Pattern.init();
    selected_pattern.push(arena, .{ .sel = .{ .tag = selected_tag } });
    ui.styleMatchAfter(selected_pattern, .{
        .color = ui.dark_theme.palette.white,
        .bg_color = ui.dark_theme.primary,
    });

    var selected_hover_pattern = ui.Pattern.init();
    selected_hover_pattern.push(arena, .{ .sel = .{ .tag = selected_tag } });
    selected_hover_pattern.push(arena, .{ .op = .And, .sel = .{ .status = .{ .hover = true } } });
    ui.styleMatchAfter(selected_hover_pattern, .{
        .bg_color = ui.dark_theme.primary_hover,
    });

    var selected_active_pattern = ui.Pattern.init();
    selected_active_pattern.push(arena, .{ .sel = .{ .tag = selected_tag } });
    selected_active_pattern.push(arena, .{ .op = .And, .sel = .{ .status = .{ .active = true } } });
    ui.styleMatchAfter(selected_active_pattern, .{
        .bg_color = ui.dark_theme.primary_active,
    });
}
