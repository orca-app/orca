/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "app.c"
#include "platform/platform_thread.h"
#include "graphics/graphics.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <errno.h>

static inline void *memz(void *buf, size_t n)
{
    return memset(buf, 0, n);
}

typedef struct x11_req
{
    oc_list_elt list_elt;
    u64 buf_len;
    u8 buf[128 - 24];
} x11_req;
static void x11_write(x11_req *req, u8 *p, u64 n)
{
  OC_ASSERT(req->buf_len + sizeof(n) <= sizeof(req->buf));
  memcpy(&req->buf[req->buf_len], p, n);
  req->buf_len += n;
}
static void x11_write_u8(x11_req *req, u8 n) { x11_write(req, (u8 *)&n, sizeof(n)); }
static void x11_write_u16(x11_req *req, u16 n) { x11_write(req, (u8 *)&n, sizeof(n)); }
static void x11_write_u32(x11_req *req, u32 n) { x11_write(req, (u8 *)&n, sizeof(n)); }
static void x11_write_skip(x11_req *req, u64 bytes)
{
    OC_ASSERT(req->buf_len + bytes <= sizeof(req->buf));
    memz(&req->buf[req->buf_len], bytes);
    req->buf_len += bytes;
}
static void x11_write_pad(x11_req *req)
{
    u64 padded = oc_align_up_pow2(req->buf_len, 4);
    OC_ASSERT(padded <= sizeof(req->buf));
    req->buf_len = padded;
}
static void x11_write_header(x11_req *req, x11_op op, u8 val)
{
    x11_write_u8(req, op);
    x11_write_u8(req, val);
    x11_write_u16(req, 0);  /* Length, to fill in with x11_set_req_len */
}
static void x11_set_req_len(x11_req *req)
{
    x11_write_pad(req);
    u16 len = req->buf_len / 4;
    memcpy(&req->buf[2], &len, sizeof(len));
}
static u64 x11_commit(oc_linux_x11 *x11, x11_req *req)
{
    // TODO(pld): Do we need concurrency safety? See later, when things start
    // working on one thread at least!
    x11_req *p = oc_arena_dup(&x11->req_arena, req, sizeof(*req));
    oc_list_push_back(&x11->req_list, &p->list_elt);
    x11->committed++;
    return x11->committed;
}

static void x11_flush(oc_linux_x11 *x11)
{
    if(oc_list_empty(x11->req_list))  return;

    /* As per the X11 protocol spec, some requests don't have replies. But we
     * still need a way to know whether the server processed the request. This
     * can be done via sending an additional dummy request which the server is
     * required to give a reply back. libx11 uses GetInputFocus for that, let's
     * do the same. We avoid doing that for the connection setup, as it may
     * fail and any subsequent reply will be non-sensical. */
    if(x11->sent > 0) {
        x11_req req = {0};
        x11_write_header(&req, X11_OP_GET_INPUT_FOCUS, 0);
        x11_set_req_len(&req);
        x11_commit(x11, &req);
    }

    u64 reqs_len = x11->committed - x11->sent;
    struct iovec *reqs_iov = oc_arena_push_array(&x11->req_arena, struct iovec, reqs_len);
    u64 i = 0;
    u64 to_send = 0;
    oc_list_for(x11->req_list, req, x11_req, list_elt)
    {
        reqs_iov[i].iov_base = req->buf;
        reqs_iov[i].iov_len = req->buf_len;
        to_send += req->buf_len;
        i++;
    }

    ssize_t sent = 0;
    do {
        sent = sendmsg(x11->sock, &(struct msghdr){
            .msg_iov = reqs_iov,
            .msg_iovlen = reqs_len,
        }, 0);
    } while(sent == -1 && errno == EINTR);
    OC_ASSERT(sent == to_send);

    oc_arena_clear(&x11->req_arena);
    oc_list_init(&x11->req_list);
    x11->sent = x11->committed;
}

typedef struct x11_res
{
    u64 buf_off;
    u64 buf_len;
    u64 buf_cap;
    u8 *buf;
    u64 id;
    oc_list_elt list_elt;
} x11_res;
static void x11_read(x11_res *res, u8 *p, u64 n)
{
    OC_ASSERT(res->buf_len - res->buf_off >= sizeof(n));
    memcpy(p, &res->buf[res->buf_off], n);
    res->buf_off += n;
}
static u8 x11_read_u8(x11_res *res) { u8 n = 0; x11_read(res, (u8 *)&n, sizeof(n)); return n; }
static u16 x11_read_u16(x11_res *res) { u16 n = 0; x11_read(res, (u8 *)&n, sizeof(n)); return n; }
static u32 x11_read_u32(x11_res *res) { u32 n = 0; x11_read(res, (u8 *)&n, sizeof(n)); return n; }
static oc_str8 x11_read_s(x11_res *res, oc_arena *arena, u64 len)
{
    u8 *p = oc_arena_push(arena, len);
    x11_read(res, p, len);
    return (oc_str8){ (char *)p, len };
}
static void x11_read_skip(x11_res *res, u64 bytes)
{
    OC_ASSERT(res->buf_len - res->buf_off >= bytes);
    res->buf_off += bytes;
}
static void x11_read_pad(x11_res *res)
{
    u64 padded = oc_align_up_pow2(res->buf_off, 4);
    OC_ASSERT(padded <= res->buf_len);
    res->buf_off = padded;
}

typedef struct x11_res_header
{
    x11_success ok;
    u32 code;  /* Error code if X11_FAILED, data byte if X11_SUCCESS. */
    u64 id;
    u64 len;  /* Bad resource ID if X11_FAILED, additional data byte-length if X11_SUCCESS */
} x11_res_header;
static x11_res_header x11_read_header(x11_res *res)
{
    x11_res_header header = {0};
    header.ok = (x11_success)x11_read_u8(res);
    header.code = (u32)x11_read_u8(res);
    header.id = (u64)x11_read_u16(res);
    header.len = (u64)x11_read_u32(res);
    return header;
}

static x11_res x11_recv(oc_linux_x11 *x11, u64 req_id)
{
    oc_list_for(x11->res_list, res, x11_res, list_elt)
    {
        if(res->id == req_id)  return *res;
    }

    u64 recv_buf_cap = 0;
    {
        int cap = 0;
        socklen_t cap_optlen = sizeof(cap);
        int ok = getsockopt(x11->sock, SOL_SOCKET, SO_RCVBUF, &cap, &cap_optlen);
        OC_ASSERT(ok == 0); // TODO(pld): log err
        OC_ASSERT(cap_optlen == sizeof(cap));
        OC_DEBUG_ASSERT(cap > 0);
        recv_buf_cap = (u64)cap;
    }
    u8 *recv_buf = oc_arena_push(&x11->res_arena, recv_buf_cap);
    u64 recv_buf_len = 0;

    x11_res *found_res = NULL;
    while(x11->received < x11->sent)
    {
        ssize_t received = recv(x11->sock, recv_buf, recv_buf_cap, 0);
        if(received == -1 && errno == EINTR)  continue;
        OC_ASSERT(received > 0);
        recv_buf_len = (u64)received;
        OC_ASSERT(recv_buf_len >= 32);

        for(u64 off = 0; off < recv_buf_len;)
        {
            x11_res res = {0};
            res.buf = &recv_buf[off];
            res.buf_len = recv_buf_len - off;
            res.buf_cap = recv_buf_cap - off;

            u64 seq_id = 0;
            u64 len = 0;
            if(x11->received == 0)
            {
                /* Connection setup, no header. */
                seq_id = 1;
                len = recv_buf_len;
            }
            else
            {
                x11_res_header header = x11_read_header(&res);
                seq_id = header.id + 1;
                len = 32 + header.len;
            }

            res = (x11_res){0};
            res.buf = oc_arena_push_zero(&x11->res_arena, len);
            memcpy(res.buf, &recv_buf[off], len);
            res.buf_len = len;
            res.buf_cap = len;
            res.id = seq_id;

            x11_res *stored_res = oc_arena_push_zero(&x11->res_arena, sizeof(res));
            memcpy(stored_res, &res, sizeof(res));
            oc_list_push_back(&x11->res_list, &stored_res->list_elt);

            x11->received = stored_res->id;
            if(stored_res->id == req_id)  found_res = stored_res;

            off += len;
        }
    }

    if(!found_res)
    {
        /* No reply for this req_id, store dummy success. */
        x11_res res = {0};
        res.buf_cap = 32;
        res.buf_len = 32;
        res.buf = oc_arena_push_zero(&x11->res_arena, 32);
        res.buf[0] = X11_SUCCESS;
        *(u16 *)&res.buf[2] = (u16)req_id;
        res.id = req_id;

        found_res = oc_arena_push_zero(&x11->res_arena, sizeof(res));
        memcpy(found_res, &res, sizeof(res));
        oc_list_push_back(&x11->res_list, &found_res->list_elt);
    }

    return *found_res;
}

static void x11_clear_recv(oc_linux_x11 *x11)
{
    oc_arena_clear(&x11->res_arena);
    oc_list_init(&x11->res_list);
}

static u32 x11_gen_id(oc_linux_x11 *x11)
{
    x11->gen_id++;
    return x11->setup.id_base |
        (x11->setup.id_mask & (x11->gen_id << __builtin_ctz(x11->setup.id_mask)));
}

void oc_init(void)
{
    memset(&oc_appData, 0, sizeof(oc_appData));
    oc_linux_app_data *linux = &oc_appData.linux;

    oc_init_common();

    oc_arena_scope scratch = oc_scratch_begin();

    oc_linux_x11 *x11 = &linux->x11;
    oc_arena_init(&x11->arena);
    oc_arena_init(&x11->req_arena);
    oc_arena_init(&x11->res_arena);
    oc_list_init(&x11->req_list);
    oc_list_init(&x11->res_list);

    x11->sock = socket(AF_UNIX, SOCK_STREAM, 0);
    OC_ASSERT(x11->sock != -1);

    // TODO(pld): DISPLAY env var
    const char x11_sun_path[] = "/tmp/.X11-unix/X0";
    struct sockaddr_un sun_addr = {0};
    sun_addr.sun_family = AF_UNIX;
    memcpy(&sun_addr.sun_path, x11_sun_path, sizeof(x11_sun_path));

    int ok = connect(x11->sock, (struct sockaddr *)&sun_addr, sizeof(sun_addr));
    OC_ASSERT(ok == 0);

    /* X11 connection setup */
    {
        x11_req req = {0};
        /* Byte order */
        static_assert(
            __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__,
            "Assumes target is little-endian"
        );
        x11_write_u8(&req, 'l');
        x11_write_skip(&req, 1);
        /* Protocol version */
        x11_write_u16(&req, 11);
        x11_write_u16(&req, 0);
        /* Authorization name & data. */
        x11_write_u16(&req, 0);
        x11_write_u16(&req, 0);
        x11_write_skip(&req, 2);
        // TODO(pld): u64 here vs u16 in X11 spec
        u64 req_id = x11_commit(x11, &req);

        x11_flush(x11);

        x11_res res = x11_recv(x11, req_id);
        u8 ok = x11_read_u8(&res);
        if(ok != X11_SUCCESS)
        {
            x11_read_skip(&res, 1);  /* Legacy reason length */
            x11_read_skip(&res, 2);  /* Major */
            x11_read_skip(&res, 2);  /* Minor */
            u64 reason_len = (u64)x11_read_u16(&res) * 4;
            oc_str8 reason = x11_read_s(&res, &x11->arena, reason_len);
            OC_ABORT(
                "Failed to establish X11 connection, server returned: %*s",
                oc_str8_lp(reason)
            );
        }

        x11_setup *setup = &x11->setup;
        x11_read_skip(&res, 1);
        setup->major = x11_read_u16(&res);
        setup->minor = x11_read_u16(&res);
        // TODO(pld): verify payload length against received
        x11_read_skip(&res, 2);  /* Setup payload length */
        setup->release = x11_read_u32(&res);
        setup->id_base = x11_read_u32(&res);
        setup->id_mask = x11_read_u32(&res);
        setup->motion_buffer_size = x11_read_u32(&res);
        u64 vendor_len = (u64)x11_read_u16(&res);
        setup->req_cap = (u64)x11_read_u16(&res);
        {
            setup->roots_len = (u64)x11_read_u8(&res);
            setup->roots = oc_arena_push_array(&x11->arena, x11_screen, setup->roots_len);
        }
        {
            setup->formats_len = (u64)x11_read_u8(&res);
            setup->formats = oc_arena_push_array(&x11->arena, x11_format, setup->formats_len);
        }
        setup->image_byte_order = (x11_image_byte_order)x11_read_u8(&res);
        setup->bitmap_bit_order = (x11_bitmap_bit_order)x11_read_u8(&res);
        setup->bitmap_scanline_unit = (u32)x11_read_u8(&res);
        setup->bitmap_scanline_pad = (u32)x11_read_u8(&res);
        setup->min_keycode = (u32)x11_read_u8(&res);
        setup->max_keycode = (u32)x11_read_u8(&res);
        x11_read_skip(&res, 4);
        setup->vendor = x11_read_s(&res, &x11->arena, vendor_len);
        x11_read_pad(&res);
        for(u64 i = 0; i < setup->formats_len; i++)
        {
            x11_format *format = &setup->formats[i];
            format->depth = (u32)x11_read_u8(&res);
            format->bpp = (u32)x11_read_u8(&res);
            format->scanline_pad_bits = (u32)x11_read_u8(&res);
            x11_read_skip(&res, 5);
        }
        for(u64 i = 0; i < setup->roots_len; i++)
        {
            x11_screen *screen = &setup->roots[i];
            screen->root = x11_read_u32(&res);
            screen->def_colormap = x11_read_u32(&res);
            screen->white_pixel = x11_read_u32(&res);
            screen->black_pixel = x11_read_u32(&res);
            screen->current_input_masks = x11_read_u32(&res);
            screen->width = (u32)x11_read_u16(&res);
            screen->height = (u32)x11_read_u16(&res);
            screen->width_mm = (u32)x11_read_u16(&res);
            screen->height_mm = (u32)x11_read_u16(&res);
            screen->min_installed_maps = (u32)x11_read_u16(&res);
            screen->max_installed_maps = (u32)x11_read_u16(&res);
            screen->root_visual = x11_read_u32(&res);
            screen->backing_stores = (x11_backing_stores)x11_read_u8(&res);
            screen->save_unders = (bool)x11_read_u8(&res);
            screen->root_depth = (u32)x11_read_u8(&res);
            {
                screen->allowed_depths_len = (u64)x11_read_u8(&res);
                screen->allowed_depths = oc_arena_push_array(&x11->arena, x11_depth, screen->allowed_depths_len);
            }
            for(u64 i = 0; i < screen->allowed_depths_len; i++)
            {
                x11_depth *depth = &screen->allowed_depths[i];
                depth->depth = (u32)x11_read_u8(&res);
                x11_read_skip(&res, 1);
                {
                    depth->visuals_len = (u64)x11_read_u16(&res);
                    depth->visuals = oc_arena_push_array(&x11->arena, x11_visual, depth->visuals_len);
                }
                x11_read_skip(&res, 4);
                for(u64 i = 0; i < depth->visuals_len; i++)
                {
                    x11_visual *visual = &depth->visuals[i];
                    visual->id = x11_read_u32(&res);
                    visual->class = (x11_visual_class)x11_read_u8(&res);
                    visual->bpp = (u32)x11_read_u8(&res);
                    visual->colormap_entries = (u32)x11_read_u16(&res);
                    visual->red_mask = x11_read_u32(&res);
                    visual->green_mask = x11_read_u32(&res);
                    visual->blue_mask = x11_read_u32(&res);
                    x11_read_skip(&res, 4);
                }
            }
        }
        x11_clear_recv(x11);
    }
    OC_ASSERT(x11->setup.roots_len > 0);

    // TODO(pld): aborts instead of asserts, with errno for relevant cases
    // TODO(pld): after the above, create window and map window
    //
    // TODO(pld): clang-format
    // TODO(pld): build w/ gcc instead of clang?
    // TODO(pld): debug why gdb gets very confused when debugging runtime, while lldb doesn't

    // TODO(pld): init keys
    //
    oc_scratch_end(scratch);
    return;
}

void oc_terminate(void)
{
    oc_terminate_common();
    assert(0 && "Unimplemented");
    return;
}
bool oc_should_quit(void)
{
    assert(0 && "Unimplemented");
    return false;
}
void oc_request_quit(void)
{
    assert(0 && "Unimplemented");
    return;
}
void oc_set_cursor(oc_mouse_cursor cursor)
{
    assert(0 && "Unimplemented");
    return;
}
void oc_pump_events(f64 timeout)
{
    assert(0 && "Unimplemented");
    return;
}

oc_window oc_window_create(oc_rect contentRect, oc_str8 title, oc_window_style style)
{
    oc_linux_app_data *linux = &oc_appData.linux;
    oc_linux_x11 *x11 = &linux->x11;

    // TODO(pld): title (requires WM integration)
    // TODO(pld): style
    /* Create window */
    u32 win_id = x11_gen_id(x11);
    {
        x11_req req = {0};
        x11_write_header(&req, X11_OP_CREATE_WINDOW, 0);
        /* window id, parent id */
        x11_write_u32(&req, win_id);
        x11_write_u32(&req, x11->setup.roots[0].root);
        /* x, y */
        x11_write_u16(&req, contentRect.x);
        x11_write_u16(&req, contentRect.y);
        /* width, height, border_width */
        x11_write_u16(&req, contentRect.w);
        x11_write_u16(&req, contentRect.h);
        x11_write_u16(&req, 1);
        /* class */
        x11_write_u16(&req, X11_WINDOW_CLASS_INPUT_OUTPUT);
        x11_write_u32(&req, X11_WINDOW_VISUAL_COPY_FROM_PARENT);
        /* attributes */
        x11_write_u32(&req, X11_WINDOW_ATTR_BACKGROUND_PIXMAP |
            X11_WINDOW_ATTR_BACKGROUND_PIXEL |
            X11_WINDOW_ATTR_BORDER_PIXMAP |
            X11_WINDOW_ATTR_BORDER_PIXEL |
            X11_WINDOW_ATTR_BIT_GRAVITY |
            X11_WINDOW_ATTR_WIN_GRAVITY |
            X11_WINDOW_ATTR_BACKING_STORE |
            X11_WINDOW_ATTR_BACKING_PLANES |
            X11_WINDOW_ATTR_BACKING_PIXEL |
            X11_WINDOW_ATTR_OVERRIDE_REDIRECT |
            X11_WINDOW_ATTR_SAVE_UNDER |
            X11_WINDOW_ATTR_EVENT_MASK |
            X11_WINDOW_ATTR_DO_NOT_PROPAGATE_MASK |
            X11_WINDOW_ATTR_COLOMAP |
            X11_WINDOW_ATTR_CURSOR |
            0);
        x11_write_u32(&req, X11_WINDOW_ATTR_BACKGROUND_PIXMAP_NONE);
        x11_write_u32(&req, 0);
        x11_write_u32(&req, X11_WINDOW_ATTR_BORDER_PIXMAP_COPY_FROM_PARENT);
        x11_write_u32(&req, 0);
        x11_write_u8(&req, X11_BIT_GRAVITY_FORGET), x11_write_pad(&req);
        x11_write_u8(&req, X11_WIN_GRAVITY_NORTH_WEST), x11_write_pad(&req);
        {
            x11_write_u8(&req, X11_BACKING_STORES_NOT_USEFUL);
            x11_write_pad(&req);
        }
        x11_write_u32(&req, 0);
        x11_write_u32(&req, 0);
        x11_write_u8(&req, 0), x11_write_pad(&req);
        x11_write_u8(&req, 0), x11_write_pad(&req);
        x11_write_u32(&req, 0);
        x11_write_u32(&req, 0);
        x11_write_u32(&req, X11_WINDOW_ATTR_COLORMAP_COPY_FROM_PARENT);
        x11_write_u32(&req, X11_WINDOW_ATTR_CURSOR_NONE);
        x11_set_req_len(&req);
        u64 req_id = x11_commit(x11, &req);

        x11_flush(x11);

        x11_res res = x11_recv(x11, req_id);
        x11_res_header header = x11_read_header(&res);
        if(header.ok != X11_SUCCESS)
        {
            OC_ABORT("Failed to create window");
        }

        x11_clear_recv(x11);
    }

    /* Map window */
    {
        x11_req req = {0};
        x11_write_header(&req, X11_OP_MAP_WINDOW, 0);
        x11_write_u32(&req, win_id);
        x11_set_req_len(&req);
        u64 req_id = x11_commit(x11, &req);

        x11_flush(x11);

        x11_res res = x11_recv(x11, req_id);
        x11_res_header header = x11_read_header(&res);
        if(header.ok != X11_SUCCESS)
        {
            OC_ABORT("Failed to map window");
        }

        x11_clear_recv(x11);
    }

    oc_window_data *window_data = oc_window_alloc();
    window_data->linux.x11_id = win_id;
    oc_window window = oc_window_handle_from_ptr(window_data);

    return window;
}

void oc_window_destroy(oc_window window)
{
    assert(0 && "Unimplemented");
    return;
}
void* oc_window_native_pointer(oc_window window)
{
    assert(0 && "Unimplemented");
    return NULL;
}

bool oc_window_should_close(oc_window window)
{
    assert(0 && "Unimplemented");
    return false;
}
void oc_window_request_close(oc_window window)
{
    assert(0 && "Unimplemented");
    return;
}
void oc_window_cancel_close(oc_window window)
{
    assert(0 && "Unimplemented");
    return;
}
bool oc_window_is_hidden(oc_window window)
{
    assert(0 && "Unimplemented");
    return false;
}
void oc_window_hide(oc_window window)
{
    assert(0 && "Unimplemented");
    return;
}
void oc_window_show(oc_window window)
{
    assert(0 && "Unimplemented");
    return;
}
void oc_window_set_title(oc_window window, oc_str8 title)
{
    assert(0 && "Unimplemented");
    return;
}
bool oc_window_is_minimized(oc_window window)
{
    assert(0 && "Unimplemented");
    return false;
}
bool oc_window_is_maximized(oc_window window)
{
    assert(0 && "Unimplemented");
    return false;
}
void oc_window_minimize(oc_window window)
{
    assert(0 && "Unimplemented");
    return;
}
void oc_window_maximize(oc_window window)
{
    assert(0 && "Unimplemented");
    return;
}
void oc_window_restore(oc_window window)
{
    assert(0 && "Unimplemented");
    return;
}
bool oc_window_has_focus(oc_window window)
{
    assert(0 && "Unimplemented");
    return false;
}
void oc_window_focus(oc_window window)
{
    assert(0 && "Unimplemented");
    return;
}
void oc_window_unfocus(oc_window window)
{
    assert(0 && "Unimplemented");
    return;
}
void oc_window_send_to_back(oc_window window)
{
    assert(0 && "Unimplemented");
    return;
}
void oc_window_bring_to_front(oc_window window)
{
    assert(0 && "Unimplemented");
    return;
}
oc_rect oc_window_get_frame_rect(oc_window window)
{
    assert(0 && "Unimplemented");
    return (oc_rect){0};
}
void oc_window_set_frame_rect(oc_window window, oc_rect rect)
{
    assert(0 && "Unimplemented");
    return;
}
oc_rect oc_window_get_content_rect(oc_window window)
{
    assert(0 && "Unimplemented");
    return (oc_rect){0};
}
void oc_window_set_content_rect(oc_window window, oc_rect rect)
{
    assert(0 && "Unimplemented");
    return;
}
void oc_window_center(oc_window window)
{
    assert(0 && "Unimplemented");
    return;
}
oc_rect oc_window_content_rect_for_frame_rect(oc_rect frameRect, oc_window_style style)
{
    assert(0 && "Unimplemented");
    return (oc_rect){0};
}
oc_rect oc_window_frame_rect_for_content_rect(oc_rect contentRect, oc_window_style style)
{
    assert(0 && "Unimplemented");
    return (oc_rect){0};
}
i32 oc_dispatch_on_main_thread_sync(oc_window main_window, oc_dispatch_proc proc, void* user)
{
    assert(0 && "Unimplemented");
    return -1;
}
void oc_clipboard_clear(void)
{
    assert(0 && "Unimplemented");
    return;
}
void oc_clipboard_set_string(oc_str8 string)
{
    assert(0 && "Unimplemented");
    return;
}
oc_str8 oc_clipboard_get_string(oc_arena* arena)
{
    assert(0 && "Unimplemented");
    return (oc_str8){0};
}
oc_str8 oc_clipboard_copy_string(oc_str8 backing)
{
    assert(0 && "Unimplemented");
    return (oc_str8){0};
}
bool oc_clipboard_has_tag(const char* tag)
{
    assert(0 && "Unimplemented");
    return false;
}
void oc_clipboard_set_data_for_tag(const char* tag, oc_str8 data)
{
    assert(0 && "Unimplemented");
    return;
}
oc_str8 oc_clipboard_get_data_for_tag(oc_arena* arena, const char* tag)
{
    assert(0 && "Unimplemented");
    return (oc_str8){0};
}
oc_file_dialog_result oc_file_dialog_for_table(oc_arena* arena, oc_file_dialog_desc* desc, oc_file_table* table)
{
    assert(0 && "Unimplemented");
    return (oc_file_dialog_result){0};
}
int oc_alert_popup(oc_str8 title, oc_str8 message, oc_str8_list options)
{
    assert(0 && "Unimplemented");
    return -1;
}
int oc_file_move(oc_str8 from, oc_str8 to)
{
    assert(0 && "Unimplemented");
    return -1;
}
int oc_file_remove(oc_str8 path)
{
    assert(0 && "Unimplemented");
    return -1;
}
int oc_directory_create(oc_str8 path)
{
    assert(0 && "Unimplemented");
    return -1;
}
