#include "orca.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/random.h>
#include <sched.h>
#include <errno.h>
typedef ssize_t isize;
typedef size_t usize;

// FIXME(pld): clock meanings?
// FIXME(pld): audit all implicit u64->i64 conversions
// FIXME(pld): path: handle escaped '/'
// FIXME(pld): path: oc_path_normalize
// FIXME(pld): thread: cond_timedwait and destroy error codes? Same for other
// helpers in threads

static void print_file(const char* pathname)
{
    int fd = open(pathname, O_RDONLY);
    OC_ASSERT(fd != -1);
    u8 buf[8192];
    isize n = 0;
    do {
        n = read(fd, buf, oc_array_size(buf));
        OC_ASSERT(n >= 0);
        dprintf(2, "%.*s", (int)n, buf);
    } while(n > 0);
    close(fd);
}

static oc_str8 read_file(oc_arena* arena, oc_str8 pathname)
{
    oc_arena_scope scratch = oc_scratch_begin_next(arena);
    int fd = open(oc_str8_to_cstring(scratch.arena, pathname), O_RDONLY);
    OC_ASSERT(fd != -1);
    oc_str8_list l = {0};
    isize n = 0;
    usize buf_cap = 0;
    u8* buf = NULL;
    do {
        if(buf_cap == 0)
        {
            buf_cap = 8192;
            buf = oc_arena_push(scratch.arena, buf_cap);
        }
        n = read(fd, buf, buf_cap);
        OC_ASSERT(n >= 0);
        oc_str8_list_push(scratch.arena, &l, oc_str8_from_buffer((u64)n, (char*)buf));
        buf_cap -= (usize)n;
        buf += n;
    } while(n > 0);
    close(fd);
    oc_str8 result = oc_str8_list_join(arena, l);
    oc_scratch_end(scratch);
    return result;
}

static bool pathname_exists(oc_arena* arena, oc_str8 pathname)
{
    oc_arena_scope scratch = oc_scratch_begin_next(arena);
    UNUSED struct stat statbuf;
    int err = stat(oc_str8_to_cstring(scratch.arena, pathname), &statbuf);
    oc_scratch_end(scratch);
    return !err;
}

UNUSED static i32 nop_thread_proc(void* p UNUSED)
{
    return 0;
}

static i32 set_flag_thread_proc(void* p)
{
    bool* flag = p;
    *flag = true;
    return 1;
}

static i64 gettid(void)
{
    return (i64)syscall(SYS_gettid);
}

static i32 gettid_thread_proc(void* p)
{
    i64* tid = p;
    __atomic_store_n(tid, (i64)gettid(), __ATOMIC_SEQ_CST);
    while(__atomic_load_n(tid, __ATOMIC_SEQ_CST));
    return 0;
}

static void nop_signal_handler(int signum UNUSED)
{
    return;
}

static i32 sleep_until_signaled_thread_proc(void* p)
{
    i64* signaled = p;
    pause();
    __atomic_store_n(signaled, *signaled + 1, __ATOMIC_SEQ_CST);
    pause();
    __atomic_store_n(signaled, *signaled + 1, __ATOMIC_SEQ_CST);
    pause();
    __atomic_store_n(signaled, (i64)gettid(), __ATOMIC_SEQ_CST);
    pause();
    return 0;
}

// https://en.wikipedia.org/wiki/Xorshift
struct xoshiro256pp_state { u64 a[4]; };
static void xoshiro256pp_state_reset(struct xoshiro256pp_state* state)
{
    isize n = getrandom(state, sizeof(state->a), 0);
    OC_ASSERT(n == sizeof(state->a));
}
static inline u64 xoshiro256pp_rol(u64 x, u64 k)
{
    return (x << k) | (x >> (64 - k));
}
static u64 xoshiro256pp(struct xoshiro256pp_state* state)
{
    u64* a = state->a;
    u64 n = xoshiro256pp_rol(a[0] + a[3], 23) + a[0];
    u64 t = a[1] << 17;
    a[2] ^= a[0];
    a[3] ^= a[1];
    a[1] ^= a[2];
    a[0] ^= a[3];
    a[2] ^= t;
    a[3] = xoshiro256pp_rol(a[3], 45);
    return n;
}
static u64 random_u64(struct xoshiro256pp_state* state)
{
    return xoshiro256pp(state);
}

#define compiler_barrier()  asm volatile("" ::: "memory")
#define THREADS  16
#define ITERATIONS_PER_THREAD  1000
#define SLEEP_RANGE_PER_ITERATION  ((u64)1e6)
static oc_mutex* m = NULL;
static i32 lock_inc_unlock_sleep_thread_proc(void* p)
{
    struct xoshiro256pp_state seed = {0};
    xoshiro256pp_state_reset(&seed);

    u64* n = p;
    for(usize i = 0; i < ITERATIONS_PER_THREAD; i++)
    {
        oc_mutex_lock(m);
        u64 tmp = *n + 1;
        compiler_barrier();
        *n = tmp;
        oc_mutex_unlock(m);
        oc_sleep_nano(random_u64(&seed) % SLEEP_RANGE_PER_ITERATION);
    }
    return 0;
}

static oc_ticket ticket_mutex = {0};
static i32 ticket_lock_cmp_inc_unlock_repeat_thread_proc(void* p)
{
    struct xoshiro256pp_state seed = {0};
    xoshiro256pp_state_reset(&seed);

    u64* n = p;
    for(usize i = 0; i < ITERATIONS_PER_THREAD; i++)
    {
        oc_ticket_lock(&ticket_mutex);
        u64 tmp = *n + 1;
        compiler_barrier();
        *n = tmp;
        oc_ticket_unlock(&ticket_mutex);
        oc_sleep_nano(random_u64(&seed) % SLEEP_RANGE_PER_ITERATION);
    }
    return 0;
}

static oc_condition* cond = NULL;
static i32 cond_wait_inc_thread_proc(void* p)
{
    u64* n = p;
    OC_ASSERT(oc_mutex_lock(m) == 0);
    *n += 1;
    OC_ASSERT(oc_condition_wait(cond, m) == 0);
    *n += 1;
    OC_ASSERT(oc_mutex_unlock(m) == 0);
    return 0;
}

int main(void)
{
    // platform_debug
    {
        oc_log_set_level(OC_LOG_LEVEL_INFO);
        oc_log_set_output(OC_LOG_DEFAULT_OUTPUT);
        oc_log_info("debug: Logs are working\n");
        if(0)  OC_ABORT("debug: Testing abort");
        if(0)  OC_ASSERT(0, "debug: Test assert");
    }

    // platform_clock
    {
        oc_clock_init();
        f64 a = 0.0, b = 0.0;
        a = oc_clock_time(OC_CLOCK_MONOTONIC);
        b = oc_clock_time(OC_CLOCK_MONOTONIC);
        OC_ASSERT(a < b);
        oc_log_info("clock: monotonic: %f\n", a);
        a = oc_clock_time(OC_CLOCK_UPTIME);
        b = oc_clock_time(OC_CLOCK_UPTIME);
        OC_ASSERT(a < b);
        oc_log_info("clock: uptime:    %f\n", a);
        a = oc_clock_time(OC_CLOCK_DATE);
        b = oc_clock_time(OC_CLOCK_DATE);
        OC_ASSERT(a <= b);
        oc_log_info("clock: date:      %f\n", a);
    }

    // platform_memory
    {
        const char* statm = "/proc/self/statm";
        oc_base_allocator* base = oc_base_allocator_default();
        oc_log_info("memory: initial:         "), print_file(statm);
        u8* p = oc_base_reserve(base, 1 << 30);
        oc_log_info("memory: reserved:        "), print_file(statm);
        oc_base_commit(base, p, 1 << 30);
        oc_log_info("memory: committed:       "), print_file(statm);
        p[0] = 1;
        oc_log_info("memory: wrote a page:    "), print_file(statm);
        oc_base_decommit(base, p, 1 << 30);
        oc_log_info("memory: decommitted:     "), print_file(statm);
        oc_base_commit(base, p, 1 << 30);
        oc_log_info("memory: committed:       "), print_file(statm);
        OC_ASSERT(p[0] == 0);
        for(usize n = 0; n < (1 << 30); n += 4096)  p[n] = 1;
        oc_log_info("memory: wrote all pages: "), print_file(statm);
        oc_base_decommit(base, p, 1 << 30);
        oc_log_info("memory: decommitted:     "), print_file(statm);
        oc_base_decommit(base, p, 1 << 30);
        oc_base_commit(base, p, 1 << 30);
        OC_ASSERT(p[0] == 0);
        oc_base_release(base, p, 1 << 30);
        oc_log_info("memory: released:        "), print_file(statm);
    }

    // platform_path
    {
        oc_arena_scope scratch = oc_scratch_begin();

        {
            oc_str8 path = OC_STR8_LIT("/tmp/a/b/c");
            oc_str8 dir = oc_path_slice_directory(path);
            OC_ASSERT(oc_str8_eq(dir, OC_STR8("/tmp/a/b")));
            oc_str8 filename = oc_path_slice_filename(path);
            OC_ASSERT(oc_str8_eq(filename, OC_STR8("c")));
            oc_str8_list split = oc_path_split(scratch.arena, path);
            static oc_str8 components[] = {
                OC_STR8_LIT("/"),
                OC_STR8_LIT("tmp"),
                OC_STR8_LIT("a"),
                OC_STR8_LIT("b"),
                OC_STR8_LIT("c"),
            };
            usize i = 0;
            oc_str8_list_for(split, component) {
                OC_ASSERT(oc_str8_eq(component->string, components[i]));
                i++;
            }
            OC_ASSERT(i == oc_array_size(components));
            OC_ASSERT(oc_str8_eq(oc_path_join(scratch.arena, split), OC_STR8("/tmp/a/b/c")));
        }

        {
            // TODO(pld): handle more than one trailing slash
            oc_str8 path = OC_STR8_LIT("/tmp/a/b/c/");
            oc_str8 dir = oc_path_slice_directory(path);
            OC_ASSERT(oc_str8_eq(dir, OC_STR8("/tmp/a/b")));
            oc_str8 filename = oc_path_slice_filename(path);
            OC_ASSERT(oc_str8_eq(filename, OC_STR8("c")));
            oc_str8_list split = oc_path_split(scratch.arena, path);
            static oc_str8 components[] = {
                OC_STR8_LIT("/"),
                OC_STR8_LIT("tmp"),
                OC_STR8_LIT("a"),
                OC_STR8_LIT("b"),
                OC_STR8_LIT("c"),
            };
            usize i = 0;
            oc_str8_list_for(split, component) {
                OC_ASSERT(oc_str8_eq(component->string, components[i]));
                i++;
            }
            OC_ASSERT(i == oc_array_size(components));
            OC_ASSERT(oc_str8_eq(oc_path_join(scratch.arena, split), OC_STR8("/tmp/a/b/c")));
        }

        {
            // TODO(pld): trim extraneous slashes?
            oc_str8 path = OC_STR8_LIT("/tmp/a/b/c/");
            oc_str8 path2 = oc_path_append(scratch.arena, path, OC_STR8("d/e/////f"));
            OC_ASSERT(oc_str8_eq(path2, OC_STR8("/tmp/a/b/c/d/e/////f")));
        }
        OC_ASSERT(oc_path_is_absolute(OC_STR8("/tmp/a/b/c")));
        OC_ASSERT(!oc_path_is_absolute(OC_STR8("../hello")));
        OC_ASSERT(oc_str8_eq(oc_path_executable(scratch.arena), OC_STR8("/tmp/main")));
        OC_ASSERT(oc_str8_eq(oc_path_canonical(scratch.arena, OC_STR8("../../../../bin/bash")), OC_STR8("/bin/bash")));
        OC_ASSERT(oc_str8_eq(oc_path_executable_relative(scratch.arena, OC_STR8("../bin/bash")), OC_STR8("/tmp/../bin/bash")));

        oc_scratch_end(scratch);
    }

    // platform_thread
    {
        bool flag = false;
        oc_thread* thd = oc_thread_create(set_flag_thread_proc, &flag);
        OC_ASSERT(thd);
        i64 exitCode = 0;
        int err = oc_thread_join(thd, &exitCode);
        OC_ASSERT(!err);
        OC_ASSERT(exitCode == 1);
        OC_ASSERT(flag);

        oc_str8 name = OC_STR8("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
        OC_ASSERT(name.len >= 16);
        i64 tid = -1;
        thd = oc_thread_create_with_name(gettid_thread_proc, &tid, name);
        OC_ASSERT(thd);
        OC_ASSERT(oc_str8_eq(oc_thread_get_name(thd), name));
        {
            oc_sleep_nano(100e6);
            oc_arena_scope scratch = oc_scratch_begin();
            i64 pid = (i64)getpid();
            tid = __atomic_load_n(&tid, __ATOMIC_SEQ_CST);
            oc_str8 path = oc_str8_pushf(scratch.arena, "/proc/%lld/task/%lld/comm", pid, tid);
            oc_str8 comm = read_file(scratch.arena, path);
            comm = oc_str8_slice(comm, 0, comm.len - 1);
            OC_ASSERT(oc_str8_eq(comm, oc_str8_slice(name, 0, 15)));
            oc_scratch_end(scratch);
        }
        __atomic_store_n(&tid, 0, __ATOMIC_SEQ_CST);
        OC_ASSERT(oc_thread_unique_id(thd));
        OC_ASSERT(oc_thread_self_id() != oc_thread_unique_id(thd));
        OC_ASSERT(oc_thread_join(thd, NULL) == 0);

        sigaction(SIGUSR1, &(struct sigaction){ .sa_handler = nop_signal_handler }, NULL);
        i64 signaled = 0;
        thd = oc_thread_create(sleep_until_signaled_thread_proc, &signaled);
        OC_ASSERT(thd);
        oc_sleep_nano(100e6);  // Not testing mutexes yet, use sleep as an heuristic instead
        OC_ASSERT(__atomic_load_n(&signaled, __ATOMIC_SEQ_CST) == 0);
        OC_ASSERT(oc_thread_signal(thd, SIGUSR1) == 0);
        oc_sleep_nano(100e6);
        OC_ASSERT(__atomic_load_n(&signaled, __ATOMIC_SEQ_CST) == 1);
        OC_ASSERT(oc_thread_signal(thd, SIGUSR1) == 0);
        oc_sleep_nano(100e6);
        OC_ASSERT(__atomic_load_n(&signaled, __ATOMIC_SEQ_CST) == 2);
        OC_ASSERT(oc_thread_signal(thd, SIGUSR1) == 0);
        oc_sleep_nano(100e6);
        signaled = __atomic_load_n(&signaled, __ATOMIC_SEQ_CST);
        tid = signaled;
        {
            oc_arena_scope scratch = oc_scratch_begin();
            i64 pid = (i64)getpid();
            oc_str8 path = oc_str8_pushf(scratch.arena, "/proc/%lld/task/%lld", pid, tid);
            OC_ASSERT(pathname_exists(scratch.arena, path));
            oc_scratch_end(scratch);
        }
        OC_ASSERT(oc_thread_signal(thd, SIGUSR1) == 0);
        OC_ASSERT(oc_thread_detach(thd) == 0);
        oc_sleep_nano(100e6);
        {
            oc_arena_scope scratch = oc_scratch_begin();
            i64 pid = (i64)getpid();
            oc_str8 path = oc_str8_pushf(scratch.arena, "/proc/%lld/task/%lld", pid, tid);
            OC_ASSERT(!pathname_exists(scratch.arena, path));
            oc_scratch_end(scratch);
        }

        {
            u64 n = 0;
            m = oc_mutex_create();
            OC_ASSERT(m);
            OC_ASSERT(oc_mutex_lock(m) == 0);
            OC_ASSERT(oc_mutex_unlock(m) == 0);
            oc_thread* threads[THREADS] = {0};
            for(usize i = 0; i < THREADS; i++)
            {
                threads[i] = oc_thread_create(lock_inc_unlock_sleep_thread_proc, &n);
                OC_ASSERT(threads[i]);
            }
            for(usize i = 0; i < THREADS; i++)  OC_ASSERT(oc_thread_join(threads[i], NULL) == 0);
            OC_ASSERT(oc_mutex_destroy(m) == 0);
            OC_ASSERT(n == ITERATIONS_PER_THREAD * THREADS);
        }

        {
            u64 n = 0;
            oc_ticket_init(&ticket_mutex);
            oc_ticket_lock(&ticket_mutex);
            oc_ticket_unlock(&ticket_mutex);
            oc_ticket_lock(&ticket_mutex);
            oc_thread* threads[THREADS] = {0};
            for(usize i = 0; i < THREADS; i++)
            {
                threads[i] = oc_thread_create(ticket_lock_cmp_inc_unlock_repeat_thread_proc, &n);
                OC_ASSERT(threads[i]);
            }
            oc_ticket_unlock(&ticket_mutex);
            for(usize i = 0; i < THREADS; i++)  OC_ASSERT(oc_thread_join(threads[i], NULL) == 0);
            OC_ASSERT(n == THREADS * ITERATIONS_PER_THREAD);
        }

        {
            m = oc_mutex_create();
            OC_ASSERT(m);
            cond = oc_condition_create();
            OC_ASSERT(cond);
            OC_ASSERT(oc_mutex_lock(m) == 0);
            f64 start = oc_clock_time(OC_CLOCK_MONOTONIC);
            OC_ASSERT(oc_condition_timedwait(cond, m, 0.5) == ETIMEDOUT);
            f64 end = oc_clock_time(OC_CLOCK_MONOTONIC);
            OC_ASSERT(oc_mutex_unlock(m) == 0);
            OC_ASSERT(end - start >= 0.5, "start=%f, end=%f, elapsed=%f", start, end, end - start);
            u64 n = 0;
            oc_thread* thread = oc_thread_create(cond_wait_inc_thread_proc, &n);
            OC_ASSERT(thread);
            while(n < 1);
            OC_ASSERT(n == 1);
            OC_ASSERT(oc_condition_signal(cond) == 0);
            OC_ASSERT(oc_thread_join(thread, NULL) == 0);
            OC_ASSERT(n == 2);
            n = 0;
            oc_thread* threads[THREADS] = {0};
            for(usize i = 0; i < THREADS; i++)
            {
                threads[i] = oc_thread_create(cond_wait_inc_thread_proc, &n);
                OC_ASSERT(threads[i]);
            }
            while(n < 16);
            OC_ASSERT(n == 16);
            OC_ASSERT(oc_condition_broadcast(cond) == 0);
            for(usize i = 0; i < THREADS; i++)  OC_ASSERT(oc_thread_join(threads[i], NULL) == 0);
            OC_ASSERT(n == 32);
            OC_ASSERT(oc_condition_destroy(cond) == 0);
            OC_ASSERT(oc_mutex_destroy(m) == 0);
        }
    }

    {

    }
    // TODO(pld): test platform_io
    // - oc_io_wait_single_req
    // - oc_file_nil
    // - oc_file_is_nil
    // - oc_file_open
    // - oc_file_open_at
    // - oc_file_close
    // - oc_file_pos
    // - oc_file_seek
    // - oc_file_write
    // - oc_file_read
    // - oc_file_last_error
    // - oc_file_get_status
    // - oc_file_size
    // - oc_file_open_with_request
    // TODO(pld): test platform_io_dialog
    // - oc_file_open_with_dialog
    // TODO(pld): test platform_io_internal
    // - oc_file_table_get_global
    // - oc_file_slot_alloc
    // - oc_file_slot_recycle
    // - oc_file_from_slot
    // - oc_file_slot_from_handle
    // - oc_io_wait_single_req_for_table
    // - oc_file_open_with_request_for_table
    // - oc_file_open_with_dialog_for_table
    // - oc_field_desc_nil
    // - oc_field_desc_is_nil
    // - oc_io_raw_open_at
    // - oc_io_raw_close
    // - oc_io_raw_last_error
    // - oc_io_raw_file_exists_at
    // - oc_io_raw_fstat
    // - oc_io_raw_fstat_at
    // - oc_io_raw_read_link_at

    //oc_init();
    //oc_clock_init();

    // TODO(pld): test app.h
    // - oc_init
    // - oc_terminate
    // - oc_should_quit
    // - oc_request_quit
    // - oc_set_cursor
    // - oc_pump_events
    // - oc_next_event
    // - oc_scancode_to_keycode
    // - oc_window_create
    // - oc_window_destroy
    // - oc_window_native_pointer
    // - oc_window_should_close
    // - oc_window_request_close
    // - oc_window_cancel_close
    // - oc_window_is_hidden
    // - oc_window_hide
    // - oc_window_show
    // - oc_window_set_title
    // - oc_window_is_minimized
    // - oc_window_is_maximized
    // - oc_window_minimize
    // - oc_window_maximize
    // - oc_window_restore
    // - oc_window_has_focus
    // - oc_window_focus
    // - oc_window_unfocus
    // - oc_window_send_to_back
    // - oc_window_send_to_front
    // - oc_window_get_frame_rect
    // - oc_window_set_frame_rect
    // - oc_window_set_frame_position
    // - oc_window_set_frame_size
    // - oc_window_get_content_rect
    // - oc_window_set_content_rect
    // - oc_window_set_content_position
    // - oc_window_set_content_size
    // - oc_window_center
    // - oc_window_content_rect_for_frame_rect
    // - oc_window_frame_rect_for_content_rect
    // - oc_vsync_init
    // - oc_vsync_wait
    // - oc_dispatch_on_main_thread_sync
    // - oc_clipboard_clear
    // - oc_clipboard_set_string
    // - oc_clipboard_get_string
    // - oc_clipboard_copy_string
    // - oc_clipboard_has_tag
    // - oc_clipboard_set_data_for_tag
    // - oc_clipboard_get_data_for_tag
    // - oc_file_dialog
    // - oc_file_dialog_for_table
    // - oc_alert_popup
    // - oc_file_move
    // - oc_file_remove
    // - oc_directory_create
    // - oc_window_set_size
    // TODO(pld): graphics
    // TODO(pld): text
    // TODO(pld): ui

    return 0;
}
