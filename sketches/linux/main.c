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
#include <pthread.h>
typedef ssize_t isize;

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
    return (result);
}

static bool pathname_exists(oc_arena* arena, oc_str8 pathname)
{
    oc_arena_scope scratch = oc_scratch_begin_next(arena);
    UNUSED struct stat statbuf;
    int err = stat(oc_str8_to_cstring(scratch.arena, pathname), &statbuf);
    oc_scratch_end(scratch);
    return (!err);
}

UNUSED static i32 nop_thread_proc(void* p UNUSED)
{
    return (0);
}

static i32 set_flag_thread_proc(void* p)
{
    bool* flag = p;
    *flag = true;
    return (1);
}

static pid_t gettid(void)
{
    return (pid_t)syscall(SYS_gettid);
}

static i32 gettid_thread_proc(void* p)
{
    i64* tid = p;
    __atomic_store_n(tid, (i64)gettid(), __ATOMIC_SEQ_CST);
    while(__atomic_load_n(tid, __ATOMIC_SEQ_CST));
    return (0);
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
    return (0);
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
    return (n);
}
static u64 random_u64(struct xoshiro256pp_state* state)
{
    return (xoshiro256pp(state));
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
    return (0);
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
    return (0);
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
    return (0);
}

static void pump_events_for_secs(f64 secs)
{
    while(secs > 0)
    {
        f64 start = oc_clock_time(OC_CLOCK_MONOTONIC);
        oc_pump_events(secs);
        f64 end = oc_clock_time(OC_CLOCK_MONOTONIC);
        secs -= end - start;
    }
}


static pthread_barrier_t getClipboardBarrier = {0};
static _Atomic(bool) getClipboardDone = false;
i32 get_clipboard_thread(void* user)
{
    oc_str8* s = user;
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 s2 = oc_clipboard_get_string(scratch.arena);
    OC_ASSERT(oc_str8_eq(*s, s2));
    oc_scratch_end(scratch);
    int serial = pthread_barrier_wait(&getClipboardBarrier);
    OC_ASSERT(serial == 0 || serial == PTHREAD_BARRIER_SERIAL_THREAD);
    if(serial == PTHREAD_BARRIER_SERIAL_THREAD)
    {
        atomic_store(&getClipboardDone, true);
    }
    return 0;
}

static pthread_barrier_t getFrameRectBarrier = {0};
static _Atomic(bool) getFrameRectDone = false;
typedef struct get_frame_rect_thread_user
{
    oc_window window;
    oc_rect expectedFrameRect;
} get_frame_rect_thread_user;
i32 get_frame_rect_thread(void* user)
{
    get_frame_rect_thread_user* u = user;
    oc_rect rect = oc_window_get_frame_rect(u->window);
    OC_ASSERT(oc_rect_equal(rect, u->expectedFrameRect));
    int serial = pthread_barrier_wait(&getFrameRectBarrier);
    OC_ASSERT(serial == 0 || serial == PTHREAD_BARRIER_SERIAL_THREAD);
    if(serial == PTHREAD_BARRIER_SERIAL_THREAD)
    {
        atomic_store(&getFrameRectDone, true);
    }
    return 0;
}

static i32 dispatch_test(void* user)
{
    oc_log_info("running dispatch");
    return 0;
}

static void reset_keymap()
{
    char buf[128];
    snprintf(buf, sizeof(buf), "xkbcomp /tmp/orca-linux.xkbcomp :0");
    OC_ASSERT(system(buf) == 0);
}

int main(int argc, char** argv)
{
    // platform_debug
    if(1)
    {
        oc_log_set_level(OC_LOG_LEVEL_INFO);
        oc_log_set_output(OC_LOG_DEFAULT_OUTPUT);
        oc_log_info("debug: Logs are working\n");
        if(0)  OC_ABORT("debug: Testing abort");
        if(0)  OC_ASSERT(0, "debug: Test assert");
    }

    // platform_clock
    if(1)
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
    if(0)
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
    if(0)
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
        OC_ASSERT(oc_str8_eq(oc_path_executable(scratch.arena), OC_STR8("/tmp/zig-out-orca/sketches/linux")));
        OC_ASSERT(oc_str8_eq(oc_path_canonical(scratch.arena, OC_STR8("../../../../bin/bash")), OC_STR8("/bin/bash")));
        OC_ASSERT(oc_str8_eq(oc_path_executable_relative(scratch.arena, OC_STR8("../../../bin/bash")), OC_STR8("/tmp/zig-out-orca/sketches/../../../bin/bash")));

        oc_scratch_end(scratch);
    }

    // platform_thread
    if(0)
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

    #if 0
    {
        OC_ASSERT(oc_file_is_nil(oc_file_nil()));
        oc_io_cmp cmp = oc_io_wait_single_req(&(struct oc_io_req){
            .handle = oc_file_nil(),
            .op = OC_IO_OP_MAX,
        });
        OC_ASSERT(cmp.error == OC_IO_ERR_HANDLE);
        oc_file f = oc_file_open(OC_STR8("/nonexistent"), OC_FILE_ACCESS_READ | OC_FILE_ACCESS_WRITE, 0);
        OC_ASSERT(!oc_file_is_nil(f));
        OC_ASSERT(oc_file_last_error(f) == OC_IO_ERR_NO_ENTRY);
        oc_file_close(f);
        f = oc_file_open(OC_STR8("/tmp"), OC_FILE_ACCESS_READ | OC_FILE_ACCESS_WRITE, 0);
        OC_ASSERT(!oc_file_is_nil(f));
        OC_ASSERT(oc_file_last_error(f) == OC_IO_OK);
        oc_file_status status = oc_file_status(f);

    }
    #endif
    // TODO(pld): test platform_io
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
    //
    // OC_IO_OK
    // OC_IO_ERR_UNKNOWN
    // OC_IO_ERR_OP
    // OC_IO_ERR_HANDLE
    // OC_IO_ERR_PREV
    // OC_IO_ERR_ARG
    // OC_IO_ERR_PERM
    // OC_IO_ERR_SPACE
    // OC_IO_ERR_EXISTS
    // OC_IO_ERR_NOT_DIR
    // OC_IO_ERR_DIR
    // OC_IO_ERR_MAX_FILES
    // OC_IO_ERR_MAX_LINKS
    // OC_IO_ERR_PATH_LENGTH
    // OC_IO_ERR_FILE_SIZE
    // OC_IO_ERR_OVERFLOW
    // OC_IO_ERR_NOT_READY
    // OC_IO_ERR_MEM
    // OC_IO_ERR_INTERRUPT
    // OC_IO_ERR_PHYSICAL
    // OC_IO_ERR_NO_DEVICE
    // OC_IO_ERR_WALKOUT
    //
    // OC_FILE_OPEN_APPEND
    // OC_FILE_OPEN_TRUNCATE
    // OC_FILE_OPEN_CREATE
    // OC_FILE_OPEN_SYMLINK
    // OC_FILE_OPEN_NO_FOLLOW
    // OC_FILE_OPEN_RESTRICT
    //
    // OC_FILE_ACCESS_READ
    // OC_FILE_ACCESS_WRITE
    //
    // OC_FILE_SEEK_SET
    // OC_FILE_SEEK_END
    // OC_FILE_SEEK_CURRENT
    //
    // OC_FILE_REGULAR
    // OC_FILE_DIRECTORY
    // OC_FILE_SYMLINK
    // OC_FILE_BLOCK
    // OC_FILE_CHARACTER
    // OC_FILE_FIFO
    // OC_FILE_SOCKET
    //
    // OC_FILE_OTHER_EXEC
    // OC_FILE_OTHER_WRITE
    // OC_FILE_OTHER_READ
    // OC_FILE_GROUP_EXEC
    // OC_FILE_GROUP_WRITE
    // OC_FILE_GROUP_READ
    // OC_FILE_OWNER_EXEC
    // OC_FILE_OWNER_WRITE
    // OC_FILE_OWNER_READ
    // OC_FILE_STICKY_BIT
    // OC_FILE_SET_GID
    // OC_FILE_SET_UID
    //
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

    oc_set_argc_argv(argc, argv);
    oc_init();
    oc_terminate();

    oc_init();
    oc_arena_scope scratch = oc_scratch_begin();
    oc_rect rect = { 100.0f, 100.0f, 400.0f, 200.0f };
    oc_window win = {0};
    win = oc_window_create(rect, OC_STR8("Orca on Linux"), 0);
    OC_ASSERT(!oc_window_is_nil(win));
    {
        void* p = oc_window_native_pointer(win);
        OC_ASSERT(p);
        p = oc_window_native_pointer(oc_window_nil());
        OC_ASSERT(!p);
    }
    oc_window_set_title(win, OC_STR8("Orca on Linux edited"));

    OC_ASSERT(oc_window_is_hidden(win));

    f64 baseTimeout = 5.0;
    #define CHECK4(expr, stat, post, kill)  \
        do  \
        { \
            f64 timeout = baseTimeout;  \
            bool ok = false;  \
            bool first = true;  \
            while(!ok && timeout > 0.0)  \
            {  \
                f64 start = oc_clock_time(OC_CLOCK_MONOTONIC);  \
                oc_pump_events(first ? 0.0 : timeout);  \
                f64 elapsed = oc_clock_time(OC_CLOCK_MONOTONIC) - start;  \
                timeout -= elapsed;  \
                (stat);  \
                ok = (expr);  \
                (post);  \
                first = false;  \
            }  \
            oc_log_info("CHECK\n  expr: %s\n  stat: %s\n  post: %s\n  kill: %s\n  took: %fs\n", #expr, #stat, #post, #kill, baseTimeout - timeout);  \
            if(kill)  OC_ASSERT(ok);  \
        }  \
        while(0)
    #define CHECK3(expr, stat, kill)  CHECK4(expr, stat, (void)0, kill)
    #define CHECK2(expr, stat)  CHECK3(expr, stat, true)
    #define CHECK(expr)  CHECK2(expr, (void)0)

    #define CHECK_EV2(exp_type, exp_win, kill)  \
        __extension__({  \
            oc_event* _ev = NULL;  \
            while(!(_ev && _ev->type == (exp_type) && _ev->window.h == (exp_win).h) && (_ev = oc_next_event(scratch.arena)))  \
            {  \
                if(1)  oc_log_info("CHECK_EV: got ev: type=%d, win=%llx\n", _ev->type, _ev->window.h);  \
            }  \
            if(kill)  OC_ASSERT(_ev);  \
            _ev;  \
        })
    #define CHECK_EV(exp_type, exp_win)  CHECK_EV2(exp_type, exp_win, true)

    // Withdrawn -> Normal
    oc_window_show(win);
    CHECK(!oc_window_is_hidden(win) && !oc_window_is_minimized(win));
    CHECK_EV(OC_EVENT_WINDOW_SHOW, win);

    // Normal -> Iconic
    oc_window_minimize(win);
    CHECK(!oc_window_is_hidden(win) && oc_window_is_minimized(win));
    CHECK_EV(OC_EVENT_WINDOW_HIDE, win);

    // Iconic -> Normal
    oc_window_show(win);
    CHECK(!oc_window_is_hidden(win) && !oc_window_is_minimized(win));
    CHECK_EV(OC_EVENT_WINDOW_SHOW, win);

    // Normal -> Withdrawn
    oc_window_hide(win);
    CHECK(oc_window_is_hidden(win) && !oc_window_is_minimized(win));
    CHECK_EV(OC_EVENT_WINDOW_HIDE, win);

    // Withdrawn -> Iconic
    oc_window_minimize(win);
    CHECK(!oc_window_is_hidden(win) && oc_window_is_minimized(win));
    {
        oc_event* ev = NULL;
        bool got = false;
        while(!got && (ev = oc_next_event(scratch.arena)))
        {
            got = (ev->type == OC_EVENT_WINDOW_SHOW || ev->type == OC_EVENT_WINDOW_HIDE) && ev->window.h == win.h;
        }
        OC_ASSERT(!got);
    }

    // Iconic -> Withdrawn
    oc_window_hide(win);
    CHECK(oc_window_is_hidden(win) && !oc_window_is_minimized(win));
    {
        oc_event* ev = NULL;
        bool got = false;
        while(!got && (ev = oc_next_event(scratch.arena)))
        {
            got = (ev->type == OC_EVENT_WINDOW_SHOW || ev->type == OC_EVENT_WINDOW_HIDE) && ev->window.h == win.h;
        }
        OC_ASSERT(!got);
    }

    // Withdrawn -> Normal, again
    oc_window_show(win);
    CHECK(!oc_window_is_hidden(win) && !oc_window_is_minimized(win));
    CHECK_EV(OC_EVENT_WINDOW_SHOW, win);

    // Normal -> Maximized
    oc_window_maximize(win);
    CHECK(!oc_window_is_hidden(win) && !oc_window_is_minimized(win) && oc_window_is_maximized(win));
    {
        oc_event* ev = NULL;
        bool got = false;
        while(!got && (ev = oc_next_event(scratch.arena)))
        {
            got = (ev->type == OC_EVENT_WINDOW_SHOW || ev->type == OC_EVENT_WINDOW_HIDE) && ev->window.h == win.h;
        }
        OC_ASSERT(!got);
    }

    // Maximized -> Iconic
    oc_window_minimize(win);
    CHECK(!oc_window_is_hidden(win) && oc_window_is_minimized(win) && !oc_window_is_maximized(win));
    CHECK_EV(OC_EVENT_WINDOW_HIDE, win);

    // Iconic -> Maximized
    oc_window_maximize(win);
    CHECK(!oc_window_is_hidden(win) && !oc_window_is_minimized(win) && oc_window_is_maximized(win));
    CHECK_EV(OC_EVENT_WINDOW_SHOW, win);

    // Maximized -> Withdrawn
    oc_window_hide(win);
    CHECK(oc_window_is_hidden(win) && !oc_window_is_minimized(win) && !oc_window_is_maximized(win));
    CHECK_EV(OC_EVENT_WINDOW_HIDE, win);

    // Withdrawn -> Maximized
    oc_window_maximize(win);
    CHECK(!oc_window_is_hidden(win) && !oc_window_is_minimized(win) && oc_window_is_maximized(win));
    CHECK_EV(OC_EVENT_WINDOW_SHOW, win);

    // Maximized -> Normal
    oc_window_show(win);
    CHECK(!oc_window_is_hidden(win) && !oc_window_is_minimized(win) && oc_window_is_maximized(win));
    {
        oc_event* ev = NULL;
        bool got = false;
        while(!got && (ev = oc_next_event(scratch.arena)))
        {
            got = (ev->type == OC_EVENT_WINDOW_SHOW || ev->type == OC_EVENT_WINDOW_HIDE) && ev->window.h == win.h;
        }
        OC_ASSERT(!got);
    }

    // Normal -> Restored
    oc_window_restore(win);
    CHECK(!oc_window_is_hidden(win) && !oc_window_is_minimized(win) && !oc_window_is_maximized(win));
    {
        oc_event* ev = NULL;
        bool got = false;
        while(!got && (ev = oc_next_event(scratch.arena)))
        {
            got = (ev->type == OC_EVENT_WINDOW_SHOW || ev->type == OC_EVENT_WINDOW_HIDE) && ev->window.h == win.h;
        }
        OC_ASSERT(!got);
    }

    // Restored -> Maximized
    oc_window_maximize(win);
    CHECK(!oc_window_is_hidden(win) && !oc_window_is_minimized(win) && oc_window_is_maximized(win));
    {
        oc_event* ev = NULL;
        bool got = false;
        while(!got && (ev = oc_next_event(scratch.arena)))
        {
            got = (ev->type == OC_EVENT_WINDOW_SHOW || ev->type == OC_EVENT_WINDOW_HIDE) && ev->window.h == win.h;
        }
        OC_ASSERT(!got);
    }

    // Maximized -> Restored
    oc_window_restore(win);
    CHECK(!oc_window_is_hidden(win) && !oc_window_is_minimized(win) && !oc_window_is_maximized(win));
    {
        oc_event* ev = NULL;
        bool got = false;
        while(!got && (ev = oc_next_event(scratch.arena)))
        {
            got = (ev->type == OC_EVENT_WINDOW_SHOW || ev->type == OC_EVENT_WINDOW_HIDE) && ev->window.h == win.h;
        }
        OC_ASSERT(!got);
    }

    // Restored -> Iconic
    oc_window_minimize(win);
    CHECK(!oc_window_is_hidden(win) && oc_window_is_minimized(win) && !oc_window_is_maximized(win));
    CHECK_EV(OC_EVENT_WINDOW_HIDE, win);

    // Iconic -> Restored
    oc_window_restore(win);
    CHECK(!oc_window_is_hidden(win) && !oc_window_is_minimized(win) && !oc_window_is_maximized(win));
    CHECK_EV(OC_EVENT_WINDOW_SHOW, win);

    // Restored -> Withdrawn
    oc_window_hide(win);
    CHECK(oc_window_is_hidden(win) && !oc_window_is_minimized(win) && !oc_window_is_maximized(win));
    CHECK_EV(OC_EVENT_WINDOW_HIDE, win);

    // Withdrawn -> Restored
    oc_window_restore(win);
    CHECK(!oc_window_is_hidden(win) && !oc_window_is_minimized(win) && !oc_window_is_maximized(win));
    CHECK_EV(OC_EVENT_WINDOW_SHOW, win);

    // Restored -> Normal
    oc_window_show(win);
    CHECK(!oc_window_is_hidden(win) && !oc_window_is_minimized(win) && !oc_window_is_maximized(win));
    {
        oc_event* ev = NULL;
        bool got = false;
        while(!got && (ev = oc_next_event(scratch.arena)))
        {
            got = (ev->type == OC_EVENT_WINDOW_SHOW || ev->type == OC_EVENT_WINDOW_HIDE) && ev->window.h == win.h;
        }
        OC_ASSERT(!got);
    }

    u64 stack_pos0 = oc_linux_debug_window_stack_pos(win), stack_pos = U64_MAX;
    // ? -> back
    oc_window_send_to_back(win);
    CHECK2(stack_pos < stack_pos0, stack_pos = oc_linux_debug_window_stack_pos(win));
    stack_pos0 = stack_pos;

    // back -> front
    oc_window_bring_to_front(win);
    CHECK2(stack_pos > stack_pos0, stack_pos = oc_linux_debug_window_stack_pos(win));
    stack_pos0 = stack_pos;

    // front -> back
    oc_window_send_to_back(win);
    CHECK2(stack_pos < stack_pos0, stack_pos = oc_linux_debug_window_stack_pos(win));
    stack_pos0 = stack_pos;

    // back -> front, again
    oc_window_bring_to_front(win);
    CHECK2(stack_pos > stack_pos0, stack_pos = oc_linux_debug_window_stack_pos(win));
    stack_pos0 = stack_pos;

    rect.x += rect.w;
    oc_window win2 = {0};
    win2 = oc_window_create(rect, OC_STR8("Orca on Linux (2)"), 0);
    OC_ASSERT(!oc_window_is_nil(win2));
    OC_ASSERT(oc_window_is_hidden(win2));
    //TODO(pld): do we require a window to be shown to be focusable?

    oc_window_show(win2);
    oc_window_unfocus(win2);
    oc_window_unfocus(win);
    {
        oc_event* ev = NULL;
        CHECK3(ev, ev = oc_next_event(scratch.arena), false);
    }

    // ? -> 1st focused
    oc_window_focus(win);
    CHECK(oc_window_has_focus(win) && !oc_window_has_focus(win2));
    CHECK_EV(OC_EVENT_WINDOW_FOCUS, win);

    // 1st focused -> unfocused
    oc_window_unfocus(win);
    CHECK(!oc_window_has_focus(win) && !oc_window_has_focus(win2));
    CHECK_EV(OC_EVENT_WINDOW_UNFOCUS, win);

    // unfocused -> 2nd focused
    oc_window_focus(win2);
    CHECK(!oc_window_has_focus(win) && oc_window_has_focus(win2));
    CHECK_EV(OC_EVENT_WINDOW_FOCUS, win2);

    // 2nd focused -> 1st focused
    oc_window_focus(win);
    CHECK(oc_window_has_focus(win) && !oc_window_has_focus(win2));
    CHECK_EV(OC_EVENT_WINDOW_UNFOCUS, win2);
    CHECK_EV(OC_EVENT_WINDOW_FOCUS, win);

    // 1st focused -> 2nd focused
    oc_window_focus(win2);
    CHECK(!oc_window_has_focus(win) && oc_window_has_focus(win2));
    CHECK_EV(OC_EVENT_WINDOW_UNFOCUS, win);
    CHECK_EV(OC_EVENT_WINDOW_FOCUS, win2);

    // 2nd focused -> unfocused
    oc_window_unfocus(win2);
    CHECK(!oc_window_has_focus(win) && !oc_window_has_focus(win2));
    CHECK_EV(OC_EVENT_WINDOW_UNFOCUS, win2);

    oc_window_destroy(win2);

    OC_ASSERT(!oc_window_is_hidden(win) && !oc_window_is_minimized(win));

    rect = (oc_rect){100, 100, 100, 100};
    oc_rect rect2 = {0};
    oc_event* ev = NULL;
    oc_window_set_content_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_content_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.content));

    rect.x += 100;
    oc_window_set_content_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_content_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_MOVE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.content));

    rect.y += 100;
    oc_window_set_content_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_content_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_MOVE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.content));

    rect.x += 100;
    rect.y += 100;
    oc_window_set_content_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_content_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_MOVE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.content));

    rect.w += 100;
    oc_window_set_content_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_content_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.content));

    rect.h += 100;
    oc_window_set_content_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_content_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.content));

    rect.w += 100;
    rect.h += 100;
    oc_window_set_content_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_content_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.content));

    rect.x -= 100;
    rect.w -= 100;
    oc_window_set_content_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_content_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.content));

    rect.y -= 100;
    rect.h -= 100;
    oc_window_set_content_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_content_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.content));

    rect.x += 100;
    rect.y += 100;
    rect.w += 100;
    rect.h += 100;
    oc_window_set_content_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_content_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.content));

    rect.x -= 100;
    rect.y -= 100;
    rect.w -= 100;
    rect.h -= 100;
    oc_window_set_content_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_content_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.content));

    rect = (oc_rect){100, 100, 100, 100};
    rect2 = (oc_rect){0};
    oc_window_set_frame_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_frame_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.frame));

    rect.x += 100;
    oc_window_set_frame_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_frame_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_MOVE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.frame));

    rect.y += 100;
    oc_window_set_frame_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_frame_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_MOVE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.frame));

    rect.x += 100;
    rect.y += 100;
    oc_window_set_frame_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_frame_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_MOVE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.frame));

    rect.w += 100;
    oc_window_set_frame_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_frame_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.frame));

    rect.h += 100;
    oc_window_set_frame_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_frame_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.frame));

    rect.w += 100;
    rect.h += 100;
    oc_window_set_frame_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_frame_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.frame));

    rect.x -= 100;
    rect.w -= 100;
    oc_window_set_frame_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_frame_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.frame));

    rect.y -= 100;
    rect.h -= 100;
    oc_window_set_frame_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_frame_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.frame));

    rect.x += 100;
    rect.y += 100;
    rect.w += 100;
    rect.h += 100;
    oc_window_set_frame_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_frame_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.frame));

    rect.x -= 100;
    rect.y -= 100;
    rect.w -= 100;
    rect.h -= 100;
    oc_window_set_frame_rect(win, rect);
    CHECK2(oc_rect_equal(rect, rect2), rect2 = oc_window_get_frame_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.frame));

    rect = (oc_rect){100, 100, 100, 100};
    oc_window_set_frame_rect(win, rect);
    oc_window_center(win);
    oc_rect workarea = oc_linux_debug_window_workarea(win);
    oc_rect centered_rect = rect;
    centered_rect.x = workarea.x + (workarea.w - rect.w) / 2;
    centered_rect.y = workarea.y + (workarea.h - rect.h) / 2;
    rect2 = (oc_rect){0};
    CHECK2(oc_rect_equal(centered_rect, rect2), rect2 = oc_window_get_frame_rect(win));
    ev = CHECK_EV(OC_EVENT_WINDOW_RESIZE, win);
    OC_ASSERT(oc_rect_equal(rect, ev->move.frame));
    ev = CHECK_EV(OC_EVENT_WINDOW_MOVE, win);
    OC_ASSERT(oc_rect_equal(centered_rect, ev->move.frame));

    {
        rect = (oc_rect){300, 500, 150, 300};
        oc_window_set_frame_rect(win, rect);

        oc_thread* threads[16] = {0};
        OC_ASSERT(pthread_barrier_init(&getFrameRectBarrier, NULL, oc_array_size(threads)) == 0);
        get_frame_rect_thread_user u =
        {
            .window = win,
            .expectedFrameRect = rect,
        };
        for(usize i = 0; i < oc_array_size(threads); i++)
        {
            threads[i] = oc_thread_create(get_frame_rect_thread, &u);
        }
        {
            struct timespec end = {0};
            OC_ASSERT(clock_gettime(CLOCK_REALTIME, &end) == 0);
            end.tv_sec += 3;
            while(!atomic_load(&getFrameRectDone))
            {
                oc_pump_events(0);
                struct timespec ts;
                OC_ASSERT(clock_gettime(CLOCK_REALTIME, &ts) == 0);
                OC_ASSERT(ts.tv_sec < end.tv_sec || (ts.tv_sec == end.tv_sec && ts.tv_nsec <= end.tv_nsec));
            }
        }
        for(usize i = 0; i < oc_array_size(threads); i++)
        {
            i64 res = 0;
            res = oc_thread_join(threads[i], &res);
            OC_ASSERT(res == 0);
        }
    }

    oc_window_request_close(win);
    CHECK(oc_window_should_close(win));
    CHECK_EV(OC_EVENT_WINDOW_CLOSE, win);
    oc_window_cancel_close(win);
    CHECK(!oc_window_should_close(win));

    if(0)
    {
        /* _NET_WM_PING: Shouldn't display "window is not responding" message. */
        oc_window_request_close(win);
        CHECK(oc_window_should_close(win));
        CHECK_EV(OC_EVENT_WINDOW_CLOSE, win);
        CHECK(oc_window_should_close(win));
        pump_events_for_secs(10);
        CHECK(oc_window_should_close(win));
        oc_window_cancel_close(win);
        CHECK(!oc_window_should_close(win));
    }

    if(0)
    {
        /* Window styles. */
        rect = (oc_rect){ 200, 100, 100, 100 };
        oc_str8 title = OC_STR8("Orca styled window");
        win2 = oc_window_create(rect, title, OC_WINDOW_STYLE_NO_TITLE);
        oc_window_show(win2);
        while(!CHECK_EV2(OC_EVENT_WINDOW_CLOSE, win2, false))  oc_pump_events(-1);
        oc_window_destroy(win2);
        rect.x += 100;
        win2 = oc_window_create(rect, title, OC_WINDOW_STYLE_FIXED_SIZE);
        oc_window_show(win2);
        while(!CHECK_EV2(OC_EVENT_WINDOW_CLOSE, win2, false))  oc_pump_events(-1);
        oc_window_destroy(win2);
        rect.x += 100;
        win2 = oc_window_create(rect, title, OC_WINDOW_STYLE_NO_FOCUS);
        oc_window_show(win2);
        while(!CHECK_EV2(OC_EVENT_WINDOW_CLOSE, win2, false))  oc_pump_events(-1);
        oc_window_destroy(win2);
        rect.x += 100;
        win2 = oc_window_create(rect, title, OC_WINDOW_STYLE_FLOAT);
        oc_window_show(win2);
        while(!CHECK_EV2(OC_EVENT_WINDOW_CLOSE, win2, false))  oc_pump_events(-1);
        oc_window_destroy(win2);
    }

    oc_window_show(win);
    rect = (oc_rect){ 100, 100, 100, 100 };
    oc_window_set_content_rect(win, rect);
    rect2 = oc_window_frame_rect_for_content_rect(rect, 0);
    rect = oc_window_get_frame_rect(win);
    OC_ASSERT(oc_rect_equal(rect, rect2));
    rect = (oc_rect){ 300, 400, 200, 145 };
    oc_window_set_frame_rect(win, rect);
    rect2 = oc_window_content_rect_for_frame_rect(rect, 0);
    rect = oc_window_get_content_rect(win);
    OC_ASSERT(oc_rect_equal(rect, rect2));

    /* Clipboard */
    {
        oc_str8 s = OC_STR8("test orca linux");
        oc_clipboard_set_string(s);
        oc_arena_scope scratch2 = {0};
        oc_str8 s2 = {0};
        CHECK4(oc_str8_eq(s, s2),
            (scratch2 = oc_arena_scope_begin(scratch.arena), s2 = oc_clipboard_get_string(scratch2.arena)),
            oc_arena_scope_end(scratch2),
            true);
        s = OC_STR8("test orc");
        char buf[64] = {0};
        CHECK2(oc_str8_eq(s, s2), (s2 = oc_clipboard_copy_string(oc_str8_from_buffer(9, buf))));
        OC_ASSERT(s2.ptr[s2.len] == '\0');
        s = OC_STR8("another test for orca linux");
        oc_clipboard_set_string(s);
        CHECK2(oc_str8_eq(s, s2), (s2 = oc_clipboard_copy_string(oc_str8_from_buffer(sizeof(buf), buf))));
        OC_ASSERT(s2.ptr[s2.len] == '\0');
        OC_ASSERT(oc_clipboard_has_tag("TARGETS"));
        OC_ASSERT(oc_clipboard_has_tag("TIMESTAMP"));
        OC_ASSERT(oc_clipboard_has_tag("TEXT"));
        OC_ASSERT(oc_clipboard_has_tag("UTF8_STRING"));
        OC_ASSERT(!oc_clipboard_has_tag("OC_INVALID_TAG_12345678"));
        OC_ASSERT(!oc_clipboard_has_tag("CLIPBOARD"));  /* valid atom but invalid target */
        s2 = oc_clipboard_get_data_for_tag(scratch.arena, "TEXT");
        OC_ASSERT(oc_str8_eq(s, s2));
        s2 = oc_clipboard_get_data_for_tag(scratch.arena, "UTF8_STRING");
        OC_ASSERT(oc_str8_eq(s, s2));
        s2 = oc_clipboard_get_data_for_tag(scratch.arena, "TIMESTAMP");
        OC_ASSERT(s2.len == 4 && *(u32*)s2.ptr > 0);
        //oc_log_info("try copy now\n");
        //pump_events_for_secs(10);
        oc_clipboard_clear();
        s = OC_STR8("");
        CHECK2(oc_str8_eq(s, s2), (s2 = oc_clipboard_copy_string(oc_str8_from_buffer(sizeof(buf), buf))));
        OC_ASSERT(s2.ptr[s2.len] == '\0');

        s = OC_STR8("third test for orca linux clipboard");
        oc_clipboard_set_string(s);
        CHECK4(oc_str8_eq(s, s2),
            (scratch2 = oc_arena_scope_begin(scratch.arena), s2 = oc_clipboard_get_string(scratch2.arena)),
            oc_arena_scope_end(scratch2),
            true);
        oc_clipboard_set_data_for_tag("UTF8_STRING", OC_STR8("should emit a warning"));
        oc_clipboard_set_data_for_tag("text/html", OC_STR8("<p>some html</p>"));
        oc_clipboard_set_data_for_tag("text/plain;charset=utf-8", OC_STR8("فلسطين حرة"));
        s = OC_STR8("<p>some html</p>");
        CHECK4(oc_str8_eq(s, s2),
            (scratch2 = oc_arena_scope_begin(scratch.arena), s2 = oc_clipboard_get_data_for_tag(scratch2.arena, "text/html")),
            oc_arena_scope_end(scratch2),
            true);
        s = OC_STR8("فلسطين حرة");
        CHECK4(oc_str8_eq(s, s2),
            (scratch2 = oc_arena_scope_begin(scratch.arena), s2 = oc_clipboard_get_data_for_tag(scratch2.arena, "text/plain;charset=utf-8")),
            oc_arena_scope_end(scratch2),
            true);
        s = OC_STR8("<p>some html 2</p>");
        oc_clipboard_set_data_for_tag("text/html", s);
        CHECK4(oc_str8_eq(s, s2),
            (scratch2 = oc_arena_scope_begin(scratch.arena), s2 = oc_clipboard_get_data_for_tag(scratch2.arena, "text/html")),
            oc_arena_scope_end(scratch2),
            true);
        OC_ASSERT(oc_clipboard_has_tag("text/html"));
        OC_ASSERT(oc_clipboard_has_tag("text/plain;charset=utf-8"));
        oc_clipboard_clear();
        s = OC_STR8("");
        CHECK2(oc_str8_eq(s, s2), (s2 = oc_clipboard_copy_string(oc_str8_from_buffer(sizeof(buf), buf))));
        OC_ASSERT(s2.ptr[s2.len] == '\0');
        s2 = oc_clipboard_get_data_for_tag(scratch.arena, "text/html");
        OC_ASSERT(oc_str8_eq(s, s2));
        s2 = oc_clipboard_get_data_for_tag(scratch.arena, "text/plain;charset=utf-8");
        OC_ASSERT(oc_str8_eq(s, s2));
        OC_ASSERT(!oc_clipboard_has_tag("text/html"));
        OC_ASSERT(!oc_clipboard_has_tag("text/plain;charset=utf-8"));

        s = OC_STR8("orca racy clipboard test");
        oc_clipboard_set_string(s);
        CHECK4(oc_str8_eq(s, s2),
            (scratch2 = oc_arena_scope_begin(scratch.arena), s2 = oc_clipboard_get_string(scratch2.arena)),
            oc_arena_scope_end(scratch2),
            true);
        oc_thread* threads[16] = {0};
        OC_ASSERT(pthread_barrier_init(&getClipboardBarrier, NULL, oc_array_size(threads)) == 0);
        for(usize i = 0; i < oc_array_size(threads); i++)
        {
            threads[i] = oc_thread_create(get_clipboard_thread, &s);
        }
        {
            struct timespec end = {0};
            OC_ASSERT(clock_gettime(CLOCK_REALTIME, &end) == 0);
            end.tv_sec += 3;
            while(!atomic_load(&getClipboardDone))
            {
                oc_pump_events(0);
                struct timespec ts;
                OC_ASSERT(clock_gettime(CLOCK_REALTIME, &ts) == 0);
                OC_ASSERT(ts.tv_sec < end.tv_sec || (ts.tv_sec == end.tv_sec && ts.tv_nsec <= end.tv_nsec));
            }
        }
        for(usize i = 0; i < oc_array_size(threads); i++)
        {
            i64 res = 0;
            res = oc_thread_join(threads[i], &res);
            OC_ASSERT(res == 0);
        }

        /* Large data transfers support */
        if(0)
        {
            scratch2 = oc_arena_scope_begin(scratch.arena);
            u64 largeLen = 1 << 30;
            char* largeBuf = oc_arena_push(scratch2.arena, largeLen);
            u64 n = 0xDEADBEEFDEADBEEF;
            for(u64 i = 0; i < largeLen; i += 8)  *(u64*)&largeBuf[i] = n;
            s = oc_str8_from_buffer(largeLen, largeBuf);
            oc_clipboard_set_string(s);
            s2 = (oc_str8){0};
            oc_arena_scope scratch3 = {0};
            CHECK4(oc_str8_eq(s, s2),
                (scratch3 = oc_arena_scope_begin(scratch2.arena), s2 = oc_clipboard_get_string(scratch3.arena)),
                oc_arena_scope_end(scratch3),
                true);
            oc_arena_scope_end(scratch2);
        }
    }

    /* Mouse and keyboard playground */
    if(0)
    {
        rect = (oc_rect){ 100, 100, 533, 300 };
        oc_window_set_frame_rect(win, rect);
        oc_log_info("mouse mouse around now now\n");
        bool done = false;
        while(!done)
        {
            oc_pump_events(-1);
            while(!done && (ev = oc_next_event(scratch.arena)))
            {
                if(ev->type == OC_EVENT_MOUSE_BUTTON)
                {
                    oc_log_info("event: type=%d (button), window=0x%x, action=%d, button=%d, clickCount=%d\n",
                        ev->type, ev->window, ev->key.action, ev->key.button, ev->key.clickCount);
                }
                else if(ev->type == OC_EVENT_MOUSE_WHEEL)
                {
                    oc_log_info("event: type=%d (wheel), window=0x%x, delta=%f/%f\n",
                        ev->type, ev->window, ev->mouse.deltaX, ev->mouse.deltaY);
                }
                else if(0 && ev->type == OC_EVENT_MOUSE_MOVE)
                {
                    oc_log_info("event: type=%d (move), window=0x%x, pos=%f/%f, delta=%f/%f\n",
                        ev->type, ev->window, ev->mouse.x, ev->mouse.y, ev->mouse.deltaX, ev->mouse.deltaY);
                }
                else if(ev->type == OC_EVENT_MOUSE_ENTER)
                {
                    oc_log_info("event: type=%d (enter), window=0x%x, pos=%f/%f\n",
                        ev->type, ev->window, ev->mouse.x, ev->mouse.y);
                }
                else if(ev->type == OC_EVENT_MOUSE_LEAVE)
                {
                    oc_log_info("event: type=%d (leave), window=0x%x\n",
                        ev->type, ev->window);
                }
                else if(ev->type == OC_EVENT_KEYBOARD_KEY)
                {
                    oc_log_info("event: type=%d (key), window=0x%x, action=%d, scanCode=%d, keyCode=%d, mods=%d\n",
                        ev->type, ev->window, ev->key.action, ev->key.scanCode, ev->key.keyCode, ev->key.mods);
                    if(ev->key.action == OC_KEY_PRESS && ev->key.keyCode == OC_KEY_Q && ev->key.mods & OC_KEYMOD_CTRL)
                    {
                        done = true;
                    }
                }
                else if(ev->type == OC_EVENT_KEYBOARD_CHAR)
                {
                    oc_log_info("event: type=%d (char), window=0x%x, codepoint=%d, seq=%.*s\n",
                        ev->type, ev->window, ev->character.codepoint, ev->character.seqLen, ev->character.sequence);
                }
                else if(ev->type == OC_EVENT_KEYBOARD_MODS)
                {
                    oc_log_info("event: type=%d (mods), window=0x%x, mods=%d\n",
                        ev->type, ev->window, ev->key.mods);
                }
            }
        }
    }


    /* Mouse & keyboard input */
    {
        rect = (oc_rect){ 100, 100, 533, 300 };
        oc_window_set_frame_rect(win, rect);
        oc_linux_debug_fake_mouse_move(0, 0, true);
        rect = oc_window_get_content_rect(win);
        do { ev = oc_next_event(scratch.arena); } while(ev);
        ev = NULL;
        OC_ASSERT(system("xkbcomp :0 /tmp/orca-linux.xkbcomp") == 0);
        atexit(reset_keymap);

        /* Mouse enter */
        oc_linux_debug_fake_mouse_move(rect.x + rect.w - 1, rect.y + 100, true);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_MOUSE_ENTER);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->mouse.x == rect.w - 1);
        OC_ASSERT(ev->mouse.y == 100);
        OC_ASSERT(ev->mouse.deltaX == 0);
        OC_ASSERT(ev->mouse.deltaY == 0);
        OC_ASSERT(ev->mouse.mods == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_MOUSE_MOVE);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->mouse.x == rect.w - 1);
        OC_ASSERT(ev->mouse.y == 100);
        OC_ASSERT(ev->mouse.deltaX == 0);
        OC_ASSERT(ev->mouse.deltaY == 0);
        OC_ASSERT(ev->mouse.mods == 0);
        OC_ASSERT(oc_window_has_focus(win));

        /* Mouse move */
        oc_linux_debug_fake_mouse_move(-100, -50, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_MOUSE_MOVE);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->mouse.x == rect.w - 1 - 100);
        OC_ASSERT(ev->mouse.y == 50);
        OC_ASSERT(ev->mouse.deltaX == -100);
        OC_ASSERT(ev->mouse.deltaY == -50);
        OC_ASSERT(ev->mouse.mods == 0);

        /* Mouse left click */
        oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == 0);
        OC_ASSERT(ev->key.keyCode == 0);
        OC_ASSERT(ev->key.button == OC_MOUSE_LEFT);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 1);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == 0);
        OC_ASSERT(ev->key.keyCode == 0);
        OC_ASSERT(ev->key.button == OC_MOUSE_LEFT);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 1);

        /* Mouse right click */
        oc_linux_debug_fake_mouse_button(OC_MOUSE_RIGHT, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_RIGHT, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == 0);
        OC_ASSERT(ev->key.keyCode == 0);
        OC_ASSERT(ev->key.button == OC_MOUSE_RIGHT);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 1);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == 0);
        OC_ASSERT(ev->key.keyCode == 0);
        OC_ASSERT(ev->key.button == OC_MOUSE_RIGHT);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 1);

        /* Mouse middle click */
        oc_linux_debug_fake_mouse_button(OC_MOUSE_MIDDLE, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_MIDDLE, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == 0);
        OC_ASSERT(ev->key.keyCode == 0);
        OC_ASSERT(ev->key.button == OC_MOUSE_MIDDLE);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 1);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == 0);
        OC_ASSERT(ev->key.keyCode == 0);
        OC_ASSERT(ev->key.button == OC_MOUSE_MIDDLE);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 1);

        /* Double left click */
        oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, false);
        oc_sleep_nano(50e6);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, false);
        for(usize i = 1; i <= 2; i++)
        {
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->key.action == OC_KEY_PRESS);
            OC_ASSERT(ev->key.scanCode == 0);
            OC_ASSERT(ev->key.keyCode == 0);
            OC_ASSERT(ev->key.button == OC_MOUSE_LEFT);
            OC_ASSERT(ev->key.mods == 0);
            OC_ASSERT(ev->key.clickCount == i);
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
            OC_ASSERT(ev->key.scanCode == 0);
            OC_ASSERT(ev->key.keyCode == 0);
            OC_ASSERT(ev->key.button == OC_MOUSE_LEFT);
            OC_ASSERT(ev->key.mods == 0);
            OC_ASSERT(ev->key.clickCount == i);
        }

        /* Mouse triple right click (and implicitly cancel ongoing left click count) */
        oc_linux_debug_fake_mouse_button(OC_MOUSE_RIGHT, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_RIGHT, false);
        oc_sleep_nano(50e6);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_RIGHT, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_RIGHT, false);
        oc_sleep_nano(50e6);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_RIGHT, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_RIGHT, false);
        for (usize i = 1; i <= 3; i++)
        {
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->key.action == OC_KEY_PRESS);
            OC_ASSERT(ev->key.scanCode == 0);
            OC_ASSERT(ev->key.keyCode == 0);
            OC_ASSERT(ev->key.button == OC_MOUSE_RIGHT);
            OC_ASSERT(ev->key.mods == 0);
            OC_ASSERT(ev->key.clickCount == i);
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
            OC_ASSERT(ev->key.scanCode == 0);
            OC_ASSERT(ev->key.keyCode == 0);
            OC_ASSERT(ev->key.button == OC_MOUSE_RIGHT);
            OC_ASSERT(ev->key.mods == 0);
            OC_ASSERT(ev->key.clickCount == i);
        }

        /* Mouse quadruple middle click (and implicitly cancel ongoing right click count) */
        oc_linux_debug_fake_mouse_button(OC_MOUSE_MIDDLE, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_MIDDLE, false);
        oc_sleep_nano(50e6);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_MIDDLE, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_MIDDLE, false);
        oc_sleep_nano(50e6);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_MIDDLE, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_MIDDLE, false);
        oc_sleep_nano(50e6);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_MIDDLE, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_MIDDLE, false);
        for (usize i = 1; i <= 4; i++)
        {
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->key.action == OC_KEY_PRESS);
            OC_ASSERT(ev->key.scanCode == 0);
            OC_ASSERT(ev->key.keyCode == 0);
            OC_ASSERT(ev->key.button == OC_MOUSE_MIDDLE);
            OC_ASSERT(ev->key.mods == 0);
            OC_ASSERT(ev->key.clickCount == i);
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
            OC_ASSERT(ev->key.scanCode == 0);
            OC_ASSERT(ev->key.keyCode == 0);
            OC_ASSERT(ev->key.button == OC_MOUSE_MIDDLE);
            OC_ASSERT(ev->key.mods == 0);
            OC_ASSERT(ev->key.clickCount == i);
        }

        /* Mouse double left click canceled by timeout */
        oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, false);
        oc_sleep_nano(300e6);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, false);
        for(usize i = 1; i <= 2; i++)
        {
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->key.action == OC_KEY_PRESS);
            OC_ASSERT(ev->key.scanCode == 0);
            OC_ASSERT(ev->key.keyCode == 0);
            OC_ASSERT(ev->key.button == OC_MOUSE_LEFT);
            OC_ASSERT(ev->key.mods == 0);
            OC_ASSERT(ev->key.clickCount == 1);
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
            OC_ASSERT(ev->key.scanCode == 0);
            OC_ASSERT(ev->key.keyCode == 0);
            OC_ASSERT(ev->key.button == OC_MOUSE_LEFT);
            OC_ASSERT(ev->key.mods == 0);
            OC_ASSERT(ev->key.clickCount == 1);
        }
        oc_sleep_nano(300e6);

        /* Mouse double left click canceled by move */
        oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, false);
        oc_sleep_nano(50e6);
        oc_linux_debug_fake_mouse_move(5, 5, false);
        oc_sleep_nano(50e6);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, true);
        oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, false);
        oc_sleep_nano(50e6);
        oc_linux_debug_fake_mouse_move(-5, -5, false);
        for(usize i = 1; i <= 2; i++)
        {
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->key.action == OC_KEY_PRESS);
            OC_ASSERT(ev->key.scanCode == 0);
            OC_ASSERT(ev->key.keyCode == 0);
            OC_ASSERT(ev->key.button == OC_MOUSE_LEFT);
            OC_ASSERT(ev->key.mods == 0);
            OC_ASSERT(ev->key.clickCount == 1);
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
            OC_ASSERT(ev->key.scanCode == 0);
            OC_ASSERT(ev->key.keyCode == 0);
            OC_ASSERT(ev->key.button == OC_MOUSE_LEFT);
            OC_ASSERT(ev->key.mods == 0);
            OC_ASSERT(ev->key.clickCount == 1);
            if(i == 1)
            {
                CHECK(ev = oc_next_event(scratch.arena));
                OC_ASSERT(ev->type == OC_EVENT_MOUSE_MOVE);
                OC_ASSERT(ev->window.h == win.h);
                OC_ASSERT(ev->mouse.x == rect.w - 1 - 100 + 5);
                OC_ASSERT(ev->mouse.y == 50 + 5);
                OC_ASSERT(ev->mouse.deltaX == 5);
                OC_ASSERT(ev->mouse.deltaY == 5);
                OC_ASSERT(ev->mouse.mods == 0);
            }
            else if(i == 2)
            {
                CHECK(ev = oc_next_event(scratch.arena));
                OC_ASSERT(ev->type == OC_EVENT_MOUSE_MOVE);
                OC_ASSERT(ev->window.h == win.h);
                OC_ASSERT(ev->mouse.x == rect.w - 1 - 100);
                OC_ASSERT(ev->mouse.y == 50);
                OC_ASSERT(ev->mouse.deltaX == -5);
                OC_ASSERT(ev->mouse.deltaY == -5);
                OC_ASSERT(ev->mouse.mods == 0);
            }
            else
            {
                oc_unreachable();
            }
        }

        /* Mouse wheel up */
        oc_linux_debug_fake_mouse_wheel(OC_LINUX_DEBUG_WHEEL_UP, 1);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_MOUSE_WHEEL);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->mouse.deltaX == 0);
        OC_ASSERT(ev->mouse.deltaY == -120);
        OC_ASSERT(ev->mouse.mods == 0);

        /* Mouse wheel down x2 */
        oc_linux_debug_fake_mouse_wheel(OC_LINUX_DEBUG_WHEEL_DOWN, 2);
        for(usize i = 0; i < 2; i++)
        {
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_WHEEL);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->mouse.deltaX == 0);
            OC_ASSERT(ev->mouse.deltaY == 120);
            OC_ASSERT(ev->mouse.mods == 0);
        }

        /* Mouse wheel left x3 */
        oc_linux_debug_fake_mouse_wheel(OC_LINUX_DEBUG_WHEEL_LEFT, 3);
        for(usize i = 0; i < 3; i++)
        {
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_WHEEL);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->mouse.deltaX == -120);
            OC_ASSERT(ev->mouse.deltaY == 0);
            OC_ASSERT(ev->mouse.mods == 0);
        }

        /* Mouse wheel right x10 */
        oc_linux_debug_fake_mouse_wheel(OC_LINUX_DEBUG_WHEEL_RIGHT, 10);
        for(usize i = 0; i < 10; i++)
        {
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_WHEEL);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->mouse.deltaX == 120);
            OC_ASSERT(ev->mouse.deltaY == 0);
            OC_ASSERT(ev->mouse.mods == 0);
        }

        OC_ASSERT(system("setxkbmap us") == 0);

        /* Mouse click with mods */
        // TODO(pld): On my OpenBox config, the following happen when pressing
        // the Alt modifier and clicking:
        // 1. press left alt
        // 2. xkb state update
        // 3. mouse leave
        // 4. mouse enter
        // 5. release left alt
        // 6. xkb state update
        // In effect, mouse buttons pressed with the Alt modifier are grabbed
        // by the window manager for its actions (move, resize, lower). Should
        // be the expected behaviour in applications to get the click
        // regardless of the window manager's functioning?
        oc_keymod_flags mods[] =
        {
            OC_KEYMOD_SHIFT,
            OC_KEYMOD_CTRL | OC_KEYMOD_MAIN_MODIFIER,
            //OC_KEYMOD_ALT,
            OC_KEYMOD_CMD,
            OC_KEYMOD_SHIFT | OC_KEYMOD_CTRL | OC_KEYMOD_MAIN_MODIFIER,
            //OC_KEYMOD_SHIFT | OC_KEYMOD_ALT,
            OC_KEYMOD_CTRL | OC_KEYMOD_MAIN_MODIFIER | OC_KEYMOD_CMD,
            OC_KEYMOD_SHIFT | /*OC_KEYMOD_ALT |*/ OC_KEYMOD_CMD | OC_KEYMOD_CTRL | OC_KEYMOD_MAIN_MODIFIER,
        };
        for(usize i = 0; i < oc_array_size(mods); i++)
        {
            if(mods[i] & OC_KEYMOD_SHIFT) oc_linux_debug_fake_key(OC_SCANCODE_LEFT_SHIFT, true);
            if(mods[i] & OC_KEYMOD_CTRL) oc_linux_debug_fake_key(OC_SCANCODE_LEFT_CONTROL, true);
            if(mods[i] & OC_KEYMOD_ALT) oc_linux_debug_fake_key(OC_SCANCODE_LEFT_ALT, true);
            if(mods[i] & OC_KEYMOD_CMD) oc_linux_debug_fake_key(OC_SCANCODE_LEFT_SUPER, true);
            oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, true);
            oc_linux_debug_fake_mouse_button(OC_MOUSE_LEFT, false);
            if(mods[i] & OC_KEYMOD_SHIFT) oc_linux_debug_fake_key(OC_SCANCODE_LEFT_SHIFT, false);
            if(mods[i] & OC_KEYMOD_CTRL) oc_linux_debug_fake_key(OC_SCANCODE_LEFT_CONTROL, false);
            if(mods[i] & OC_KEYMOD_ALT) oc_linux_debug_fake_key(OC_SCANCODE_LEFT_ALT, false);
            if(mods[i] & OC_KEYMOD_CMD) oc_linux_debug_fake_key(OC_SCANCODE_LEFT_SUPER, false);

            if(mods[i] & OC_KEYMOD_SHIFT)
            {
                CHECK(ev = oc_next_event(scratch.arena));
                OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
                OC_ASSERT(ev->window.h == win.h);
                OC_ASSERT(ev->key.action == OC_KEY_PRESS);
                OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_SHIFT);
                OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_SHIFT);
                OC_ASSERT(ev->key.button == 0);
                OC_ASSERT(ev->key.mods == (mods[i] & 0));
                OC_ASSERT(ev->key.clickCount == 0);
            }
            if(mods[i] & OC_KEYMOD_CTRL)
            {
                CHECK(ev = oc_next_event(scratch.arena));
                OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
                OC_ASSERT(ev->window.h == win.h);
                OC_ASSERT(ev->key.action == OC_KEY_PRESS);
                OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_CONTROL);
                OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_CONTROL);
                OC_ASSERT(ev->key.button == 0);
                OC_ASSERT(ev->key.mods == (mods[i] & OC_KEYMOD_SHIFT));
                OC_ASSERT(ev->key.clickCount == 0);
            }
            if(mods[i] & OC_KEYMOD_ALT)
            {
                CHECK(ev = oc_next_event(scratch.arena));
                OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
                OC_ASSERT(ev->window.h == win.h);
                OC_ASSERT(ev->key.action == OC_KEY_PRESS);
                OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_ALT);
                OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_ALT);
                OC_ASSERT(ev->key.button == 0);
                OC_ASSERT(ev->key.mods == (mods[i] & (OC_KEYMOD_SHIFT | OC_KEYMOD_CTRL | OC_KEYMOD_MAIN_MODIFIER)));
                OC_ASSERT(ev->key.clickCount == 0);
            }
            if(mods[i] & OC_KEYMOD_CMD)
            {
                CHECK(ev = oc_next_event(scratch.arena));
                OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
                OC_ASSERT(ev->window.h == win.h);
                OC_ASSERT(ev->key.action == OC_KEY_PRESS);
                OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_SUPER);
                OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_SUPER);
                OC_ASSERT(ev->key.button == 0);
                OC_ASSERT(ev->key.mods == (mods[i] & (OC_KEYMOD_SHIFT | OC_KEYMOD_CTRL | OC_KEYMOD_MAIN_MODIFIER | OC_KEYMOD_ALT)));
                OC_ASSERT(ev->key.clickCount == 0);
            }
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->key.action == OC_KEY_PRESS);
            OC_ASSERT(ev->key.scanCode == 0);
            OC_ASSERT(ev->key.keyCode == 0);
            OC_ASSERT(ev->key.button == OC_MOUSE_LEFT);
            OC_ASSERT(ev->key.mods == mods[i]);
            OC_ASSERT(ev->key.clickCount == i + 1);
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_MOUSE_BUTTON);
            OC_ASSERT(ev->window.h == win.h);
            OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
            OC_ASSERT(ev->key.scanCode == 0);
            OC_ASSERT(ev->key.keyCode == 0);
            OC_ASSERT(ev->key.button == OC_MOUSE_LEFT);
            OC_ASSERT(ev->key.mods == mods[i]);
            OC_ASSERT(ev->key.clickCount == i + 1);
            if(mods[i] & OC_KEYMOD_SHIFT)
            {
                CHECK(ev = oc_next_event(scratch.arena));
                OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
                OC_ASSERT(ev->window.h == win.h);
                OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
                OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_SHIFT);
                OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_SHIFT);
                OC_ASSERT(ev->key.button == 0);
                OC_ASSERT(ev->key.mods == (mods[i] & (OC_KEYMOD_SHIFT | OC_KEYMOD_CTRL | OC_KEYMOD_MAIN_MODIFIER | OC_KEYMOD_ALT | OC_KEYMOD_CMD)));
                OC_ASSERT(ev->key.clickCount == 0);
            }
            if(mods[i] & OC_KEYMOD_CTRL)
            {
                CHECK(ev = oc_next_event(scratch.arena));
                OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
                OC_ASSERT(ev->window.h == win.h);
                OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
                OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_CONTROL);
                OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_CONTROL);
                OC_ASSERT(ev->key.button == 0);
                OC_ASSERT(ev->key.mods == (mods[i] & (OC_KEYMOD_CTRL | OC_KEYMOD_MAIN_MODIFIER | OC_KEYMOD_ALT | OC_KEYMOD_CMD)));
                OC_ASSERT(ev->key.clickCount == 0);
            }
            if(mods[i] & OC_KEYMOD_ALT)
            {
                CHECK(ev = oc_next_event(scratch.arena));
                OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
                OC_ASSERT(ev->window.h == win.h);
                OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
                OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_ALT);
                OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_ALT);
                OC_ASSERT(ev->key.button == 0);
                OC_ASSERT(ev->key.mods == (mods[i] & (OC_KEYMOD_ALT | OC_KEYMOD_CMD)));
                OC_ASSERT(ev->key.clickCount == 0);
            }
            if(mods[i] & OC_KEYMOD_CMD)
            {
                CHECK(ev = oc_next_event(scratch.arena));
                OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
                OC_ASSERT(ev->window.h == win.h);
                OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
                OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_SUPER);
                OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_SUPER);
                OC_ASSERT(ev->key.button == 0);
                OC_ASSERT(ev->key.mods == (mods[i] & OC_KEYMOD_CMD));
                OC_ASSERT(ev->key.clickCount == 0);
            }
        }

        /* Mouse wheel up with mods */
        oc_linux_debug_fake_key(OC_SCANCODE_RIGHT_CONTROL, true);
        oc_linux_debug_fake_mouse_wheel(OC_LINUX_DEBUG_WHEEL_UP, true);
        oc_linux_debug_fake_key(OC_SCANCODE_RIGHT_CONTROL, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_RIGHT_CONTROL);
        OC_ASSERT(ev->key.keyCode == OC_KEY_RIGHT_CONTROL);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_MOUSE_WHEEL);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->mouse.deltaX == 0);
        OC_ASSERT(ev->mouse.deltaY == -120);
        OC_ASSERT(ev->mouse.mods == (OC_KEYMOD_CTRL | OC_KEYMOD_MAIN_MODIFIER));
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_RIGHT_CONTROL);
        OC_ASSERT(ev->key.keyCode == OC_KEY_RIGHT_CONTROL);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == (OC_KEYMOD_CTRL | OC_KEYMOD_MAIN_MODIFIER));
        OC_ASSERT(ev->key.clickCount == 0);

        /* Mouse move with mods */
        oc_linux_debug_fake_key(OC_SCANCODE_RIGHT_ALT, true);
        oc_sleep_nano(50e6);
        oc_linux_debug_fake_key(OC_SCANCODE_RIGHT_SHIFT, true);
        oc_sleep_nano(50e6);
        oc_linux_debug_fake_mouse_move(10, 10, false);
        oc_sleep_nano(50e6);
        oc_linux_debug_fake_key(OC_SCANCODE_RIGHT_ALT, false);
        oc_sleep_nano(50e6);
        oc_linux_debug_fake_mouse_move(-10, -10, false);
        oc_sleep_nano(50e6);
        oc_linux_debug_fake_key(OC_SCANCODE_RIGHT_SHIFT, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_RIGHT_ALT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_RIGHT_ALT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_RIGHT_SHIFT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_RIGHT_SHIFT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_ALT);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_MOUSE_MOVE);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->mouse.x == rect.w - 1 - 100 + 10);
        OC_ASSERT(ev->mouse.y == 50 + 10);
        OC_ASSERT(ev->mouse.deltaX == 10);
        OC_ASSERT(ev->mouse.deltaY == 10);
        OC_ASSERT(ev->mouse.mods == (OC_KEYMOD_ALT | OC_KEYMOD_SHIFT));
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_RIGHT_ALT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_RIGHT_ALT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == (OC_KEYMOD_ALT | OC_KEYMOD_SHIFT));
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_MOUSE_MOVE);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->mouse.x == rect.w - 1 - 100);
        OC_ASSERT(ev->mouse.y == 50);
        OC_ASSERT(ev->mouse.deltaX == -10);
        OC_ASSERT(ev->mouse.deltaY == -10);
        OC_ASSERT(ev->mouse.mods == OC_KEYMOD_SHIFT);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_RIGHT_SHIFT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_RIGHT_SHIFT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_SHIFT);
        OC_ASSERT(ev->key.clickCount == 0);

        /* US Keyboard q */
        oc_linux_debug_fake_key(OC_SCANCODE_Q, true);
        oc_linux_debug_fake_key(OC_SCANCODE_Q, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_Q);
        OC_ASSERT(ev->key.keyCode == OC_KEY_Q);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_CHAR);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->character.codepoint == U'q');
        OC_ASSERT(!strncmp(ev->character.sequence, "q", 1));
        OC_ASSERT(ev->character.seqLen == 1);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_Q);
        OC_ASSERT(ev->key.keyCode == OC_KEY_Q);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);

        /* US Keyboard Q */
        oc_linux_debug_fake_key(OC_SCANCODE_LEFT_SHIFT, true);
        oc_linux_debug_fake_key(OC_SCANCODE_Q, true);
        oc_linux_debug_fake_key(OC_SCANCODE_Q, false);
        oc_linux_debug_fake_key(OC_SCANCODE_LEFT_SHIFT, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_SHIFT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_SHIFT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_Q);
        OC_ASSERT(ev->key.keyCode == OC_KEY_Q);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_SHIFT);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_CHAR);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->character.codepoint == U'Q');
        OC_ASSERT(!strncmp(ev->character.sequence, "Q", 1));
        OC_ASSERT(ev->character.seqLen == 1);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_Q);
        OC_ASSERT(ev->key.keyCode == OC_KEY_Q);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_SHIFT);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_SHIFT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_SHIFT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_SHIFT);
        OC_ASSERT(ev->key.clickCount == 0);

        /* US keyboard w repeat */
        oc_linux_debug_fake_key(OC_SCANCODE_W, true);
        oc_sleep_nano(1e9);
        oc_linux_debug_fake_key(OC_SCANCODE_W, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_W);
        OC_ASSERT(ev->key.keyCode == OC_KEY_W);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_CHAR);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->character.codepoint == U'w');
        OC_ASSERT(!strncmp(ev->character.sequence, "w", 1));
        OC_ASSERT(ev->character.seqLen == 1);
        while(true)
        {
            CHECK(ev = oc_next_event(scratch.arena));
            OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
            OC_ASSERT(ev->window.h == win.h);
            if(ev->key.action == OC_KEY_REPEAT)
            {
                OC_ASSERT(ev->key.scanCode == OC_SCANCODE_W);
                OC_ASSERT(ev->key.keyCode == OC_KEY_W);
                OC_ASSERT(ev->key.button == 0);
                OC_ASSERT(ev->key.mods == 0);
                OC_ASSERT(ev->key.clickCount == 0);
                CHECK(ev = oc_next_event(scratch.arena));
                OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_CHAR);
                OC_ASSERT(ev->window.h == win.h);
                OC_ASSERT(ev->character.codepoint == U'w');
                OC_ASSERT(!strncmp(ev->character.sequence, "w", 1));
                OC_ASSERT(ev->character.seqLen == 1);
            }
            else if(ev->key.action == OC_KEY_RELEASE)
            {
                OC_ASSERT(ev->key.scanCode == OC_SCANCODE_W);
                OC_ASSERT(ev->key.keyCode == OC_KEY_W);
                OC_ASSERT(ev->key.button == 0);
                OC_ASSERT(ev->key.mods == 0);
                OC_ASSERT(ev->key.clickCount == 0);
                break;
            }
            else
            {
                OC_ASSERT(0);
            }
        }

        /* US keyboard 2 */
        oc_linux_debug_fake_key(OC_SCANCODE_2, true);
        oc_linux_debug_fake_key(OC_SCANCODE_2, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_2);
        OC_ASSERT(ev->key.keyCode == OC_KEY_2);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_CHAR);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->character.codepoint == U'2');
        OC_ASSERT(!strncmp(ev->character.sequence, "2", 1));
        OC_ASSERT(ev->character.seqLen == 1);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_2);
        OC_ASSERT(ev->key.keyCode == OC_KEY_2);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);

        /* US keyboard control code C-h */
        oc_linux_debug_fake_key(OC_SCANCODE_LEFT_CONTROL, true);
        oc_linux_debug_fake_key(OC_SCANCODE_H, true);
        oc_linux_debug_fake_key(OC_SCANCODE_H, false);
        oc_linux_debug_fake_key(OC_SCANCODE_LEFT_CONTROL, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_CONTROL);
        OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_CONTROL);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_H);
        OC_ASSERT(ev->key.keyCode == OC_KEY_H);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == (OC_KEYMOD_CTRL | OC_KEYMOD_MAIN_MODIFIER));
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_CHAR);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->character.codepoint == '\b');
        OC_ASSERT(!strncmp(ev->character.sequence, "\b", 1));
        OC_ASSERT(ev->character.seqLen == 1);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_H);
        OC_ASSERT(ev->key.keyCode == OC_KEY_H);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == (OC_KEYMOD_CTRL | OC_KEYMOD_MAIN_MODIFIER));
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_CONTROL);
        OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_CONTROL);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == (OC_KEYMOD_CTRL | OC_KEYMOD_MAIN_MODIFIER));
        OC_ASSERT(ev->key.clickCount == 0);

        OC_ASSERT(system("setxkbmap fr") == 0);

        /* FR Keyboard a */
        oc_linux_debug_fake_key(OC_SCANCODE_Q, true);
        oc_linux_debug_fake_key(OC_SCANCODE_Q, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_Q);
        OC_ASSERT(ev->key.keyCode == OC_KEY_A);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_CHAR);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->character.codepoint == U'a');
        OC_ASSERT(!strncmp(ev->character.sequence, "a", 1));
        OC_ASSERT(ev->character.seqLen == 1);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_Q);
        OC_ASSERT(ev->key.keyCode == OC_KEY_A);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);

        /* FR keyboard A */
        oc_linux_debug_fake_key(OC_SCANCODE_LEFT_SHIFT, true);
        oc_linux_debug_fake_key(OC_SCANCODE_Q, true);
        oc_linux_debug_fake_key(OC_SCANCODE_Q, false);
        oc_linux_debug_fake_key(OC_SCANCODE_LEFT_SHIFT, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_SHIFT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_SHIFT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_Q);
        OC_ASSERT(ev->key.keyCode == OC_KEY_A);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_SHIFT);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_CHAR);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->character.codepoint == U'A');
        OC_ASSERT(!strncmp(ev->character.sequence, "A", 1));
        OC_ASSERT(ev->character.seqLen == 1);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_Q);
        OC_ASSERT(ev->key.keyCode == OC_KEY_A);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_SHIFT);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_SHIFT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_SHIFT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_SHIFT);
        OC_ASSERT(ev->key.clickCount == 0);

        /* FR keyboard é */
        oc_linux_debug_fake_key(OC_SCANCODE_2, true);
        oc_linux_debug_fake_key(OC_SCANCODE_2, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_2);
        OC_ASSERT(ev->key.keyCode == OC_KEY_2);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_CHAR);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->character.codepoint == U'é');
        OC_ASSERT(!strncmp(ev->character.sequence, "é", 2));
        OC_ASSERT(ev->character.seqLen == 2);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_2);
        OC_ASSERT(ev->key.keyCode == OC_KEY_2);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);

        /* FR keyboard 2 */
        oc_linux_debug_fake_key(OC_SCANCODE_LEFT_SHIFT, true);
        oc_linux_debug_fake_key(OC_SCANCODE_2, true);
        oc_linux_debug_fake_key(OC_SCANCODE_2, false);
        oc_linux_debug_fake_key(OC_SCANCODE_LEFT_SHIFT, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_SHIFT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_SHIFT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_2);
        OC_ASSERT(ev->key.keyCode == OC_KEY_2);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_SHIFT);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_CHAR);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->character.codepoint == U'2');
        OC_ASSERT(!strncmp(ev->character.sequence, "2", 1));
        OC_ASSERT(ev->character.seqLen == 1);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_2);
        OC_ASSERT(ev->key.keyCode == OC_KEY_2);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_SHIFT);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_SHIFT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_SHIFT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_SHIFT);
        OC_ASSERT(ev->key.clickCount == 0);

        /* FR keyboard ~ */
        oc_linux_debug_fake_key(OC_SCANCODE_RIGHT_ALT, true);
        oc_linux_debug_fake_key(OC_SCANCODE_2, true);
        oc_linux_debug_fake_key(OC_SCANCODE_2, false);
        oc_linux_debug_fake_key(OC_SCANCODE_RIGHT_ALT, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_RIGHT_ALT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_RIGHT_ALT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_2);
        OC_ASSERT(ev->key.keyCode == OC_KEY_2);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_CHAR);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->character.codepoint == U'~');
        OC_ASSERT(!strncmp(ev->character.sequence, "~", 1));
        OC_ASSERT(ev->character.seqLen == 1);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_2);
        OC_ASSERT(ev->key.keyCode == OC_KEY_2);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_RIGHT_ALT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_RIGHT_ALT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);

        /* FR keyboard ¢ */
        oc_linux_debug_fake_key(OC_SCANCODE_LEFT_SHIFT, true);
        oc_linux_debug_fake_key(OC_SCANCODE_RIGHT_ALT, true);
        oc_linux_debug_fake_key(OC_SCANCODE_E, true);
        oc_linux_debug_fake_key(OC_SCANCODE_E, false);
        oc_linux_debug_fake_key(OC_SCANCODE_LEFT_SHIFT, false);
        oc_linux_debug_fake_key(OC_SCANCODE_RIGHT_ALT, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_SHIFT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_SHIFT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_RIGHT_ALT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_RIGHT_ALT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_SHIFT);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_E);
        OC_ASSERT(ev->key.keyCode == OC_KEY_E);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_SHIFT);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_CHAR);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->character.codepoint == U'¢');
        OC_ASSERT(!strncmp(ev->character.sequence, "¢", 2));
        OC_ASSERT(ev->character.seqLen == 2);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_E);
        OC_ASSERT(ev->key.keyCode == OC_KEY_E);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_SHIFT);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_SHIFT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_SHIFT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == OC_KEYMOD_SHIFT);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_RIGHT_ALT);
        OC_ASSERT(ev->key.keyCode == OC_KEY_RIGHT_ALT);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);

        /* FR keyboard dead key ê */
        oc_linux_debug_fake_key(OC_SCANCODE_LEFT_BRACKET, true);
        oc_linux_debug_fake_key(OC_SCANCODE_LEFT_BRACKET, false);
        oc_linux_debug_fake_key(OC_SCANCODE_E, true);
        oc_linux_debug_fake_key(OC_SCANCODE_E, false);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_BRACKET);
        OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_BRACKET);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_LEFT_BRACKET);
        OC_ASSERT(ev->key.keyCode == OC_KEY_LEFT_BRACKET);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_PRESS);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_E);
        OC_ASSERT(ev->key.keyCode == OC_KEY_E);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_CHAR);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->character.codepoint == U'ê');
        OC_ASSERT(!strncmp(ev->character.sequence, "ê", 2));
        OC_ASSERT(ev->character.seqLen == 2);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_KEYBOARD_KEY);
        OC_ASSERT(ev->window.h == win.h);
        OC_ASSERT(ev->key.action == OC_KEY_RELEASE);
        OC_ASSERT(ev->key.scanCode == OC_SCANCODE_E);
        OC_ASSERT(ev->key.keyCode == OC_KEY_E);
        OC_ASSERT(ev->key.button == 0);
        OC_ASSERT(ev->key.mods == 0);
        OC_ASSERT(ev->key.clickCount == 0);

        /* Mouse leave */
        oc_linux_debug_fake_mouse_move(rect.x + 10, rect.y + rect.h, true);
        CHECK(ev = oc_next_event(scratch.arena));
        OC_ASSERT(ev->type == OC_EVENT_MOUSE_LEAVE);
        OC_ASSERT(ev->window.h == win.h);

        reset_keymap();
    }

    oc_request_quit();
    CHECK(oc_should_quit());
    oc_cancel_quit();
    CHECK(!oc_should_quit());
    oc_window_destroy(win);
    CHECK(CHECK_EV2(OC_EVENT_QUIT, oc_window_nil(), false));
    CHECK(!oc_should_quit());
    oc_terminate();

    // TODO(pld): test app.h
    // - graphics: x11 surface base
    // - graphics: x11 webgpu surface create/destroy/get/present
    // - graphics: x11 egl / gles surface
    // - graphics: oc_vsync_init
    //   - do all surfaces vsync themselves if one syncs?
    // - graphics: oc_vsync_wait
    // - graphics: OC_EVENT_FRAME
    // - text: just test, should work out of the box
    // - ui: just test, should work out of the box
    //
    // - document weird behaviours
    // - test tls destructors
    // - test oc_dispatch_on_main_thread_sync
    // - get_content_rect / get_frame_rect should return stable rectangle, do not wait for it to stabilise
    //
    // - check _net_wm_allowed_actions?
    // - set _net_wm_bypass_compositor?
    // - set _net_wm_full_placement?
    // - send app events:
    //   - OC_EVENT_PATHDROP
    //
    // - oc_file_dialog (os native)
    // - oc_file_dialog_for_table (os native)
    // - oc_alert_popup (os native)
    // - multiple desktops?
    // TODO(pld): io
    // TODO(pld): clock
    // - _net_wm_user_time_window?
    //
    // debug other WM/DE (w/ Xorg or XWayland where relevant):
    // - X11 KDE/KWin
    // - X11 Xfce/Xfwm
    // - X11 i3
    // - X11 dwm
    // - X11 LXQt
    // - X11 GNOME/Metacity
    // - XWayland KDE/KWin
    // - XWayland Xfce/Xfwl
    // - XWayland LXQt
    // - XWayland Sway
    // - XWayland Hyprland
    // - XWayland Mango
    // - XWayland Weston
    // - XWayland dwl
    // - XWayland COSMIC
    // - XWayland River-based WM
    // - XWayland Wayfire
    // - XWayland GNOME/Mutter
    // - MATE Desktop/Marco/Compiz
    // - Cinnamon
    // - Budgie
    // - Miracle
    // - Kylin
    // - Enlightenment
    //
    // - do not implement, part of io:
    //   - oc_file_move
    //   - oc_file_remove
    //   - oc_directory_create
    //
    // later:
    // - oc_set_cursor
    // - clipboard: get/set timeout, handle if owner/requestor dies
    // - clipboard: handle alloc errors
    // - clipboard: text/html, image/png mime targets
    // - keyboard: not convinced with OC_KEY values on non-qwerty layouts
    // - keyboard: integrate w/ X Input Extension
    // - keyboard: helper to select and only use XTEST keyboard, so we don't mess up
    //   with the core keymap when testing
    // - keyboard: should not receive xkb state when not focused?
    // - keyboard: altgr mod?
    // - keyboard: grab mods?
    // - keyboard: virtual keyboards
    // - keyboard: input methods
    // - keyboard: layouts to thoroughly test: greek, russian, ???
    // - xsettings & X Resources
    // - avoid requeueing app cmds

    oc_scratch_end(scratch);
    return (0);
}
