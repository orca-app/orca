#define OC_NO_APP_LAYER 1
#include "orca.c"

typedef enum oc_test_status
{
    OC_TEST_FAIL = 0,
    OC_TEST_SKIP,
    OC_TEST_PASS,
} oc_test_status;

typedef enum oc_test_verbosity
{
    OC_TEST_PRINT_SUMMARY = 0,   // print only final summary
    OC_TEST_PRINT_GROUP_SUMMARY, // print groups and final summary
    OC_TEST_PRINT_ALL_FAILED,    // print all failed tests
    OC_TEST_PRINT_ALL,           // print every test
} oc_test_verbosity;

typedef struct oc_test_info
{
    oc_test_verbosity verbosity;
    oc_str8 name;
    oc_str8 testName;
    oc_str8 groupName;
    bool statusSet;

    u32 failed;
    u32 passed;
    u32 skipped;

    u32 totalFailed;
    u32 totalPassed;
    u32 totalSkipped;

} oc_test_info;

static const char* oc_test_status_string[] = {
    "[FAIL]",
    "[SKIP]",
    "[PASS]",
};

static const char* oc_test_status_color_start[] = {
    "\033[38;5;9m\033[1m",
    "\033[38;5;13m\033[1m",
    "\033[38;5;10m\033[1m",
};

static const char* oc_test_status_color_stop = "\033[m";

void oc_test_mark_fmt(oc_test_info* info, oc_test_status status, const char* fmt, ...)
{
    OC_ASSERT(info->testName.len, "this should be called within a oc_test_begin/end block.");
    info->statusSet = true;

    switch(status)
    {
        case OC_TEST_FAIL:
            info->failed++;
            info->totalFailed++;
            break;
        case OC_TEST_SKIP:
            info->skipped++;
            info->totalSkipped++;
            break;
        case OC_TEST_PASS:
            info->passed++;
            info->totalPassed++;
            break;
    }

    if(info->verbosity >= OC_TEST_PRINT_ALL)
    {
        printf("%s", oc_test_status_color_start[status]);
        printf("%s", oc_test_status_string[status]);
        printf("%s", oc_test_status_color_stop);

        printf(" %.*s", oc_str8_ip(info->testName));

        oc_arena_scope scratch = oc_scratch_begin();

        va_list ap;
        va_start(ap, fmt);
        oc_str8 note = oc_str8_pushfv(scratch.arena, fmt, ap);
        va_end(ap);

        if(note.len)
        {
            printf(": %.*s", oc_str8_ip(note));
        }
        printf("\n");

        oc_scratch_end(scratch);
    }
}

#define _oc_test_mark_(info, status, fmt, ...) oc_test_mark_fmt(info, status, fmt, ##__VA_ARGS__)
#define oc_test_mark(info, status, ...) _oc_test_mark_(info, status, OC_VA_NOPT("", ##__VA_ARGS__) OC_ARG1(__VA_ARGS__) OC_VA_COMMA_TAIL(__VA_ARGS__))

#define oc_test_pass(info, ...) oc_test_mark(info, OC_TEST_PASS, ##__VA_ARGS__)
#define oc_test_fail(info, ...) oc_test_mark(info, OC_TEST_FAIL, ##__VA_ARGS__)
#define oc_test_skip(info, ...) oc_test_mark(info, OC_TEST_SKIP, ##__VA_ARGS__)

void oc_test_begin(oc_test_info* info, const char* name)
{
    OC_ASSERT(info->testName.len == 0, "Test blocks should not be nested");
    info->testName = OC_STR8(name);
    info->statusSet = false;
}

void oc_test_end(oc_test_info* info)
{
    OC_ASSERT(info->statusSet, "status should be set within test block");
    info->testName = (oc_str8){};
    info->statusSet = false;
}

#define oc_test(info, name) oc_defer_loop(oc_test_begin(info, name), oc_test_end(info))

void oc_test_group_begin(oc_test_info* info, const char* name)
{
    OC_ASSERT(info->testName.len == 0, "Test groups should not be nested inside test blocks");
    OC_ASSERT(info->groupName.len == 0, "Test groups should not be nested");

    info->passed = 0;
    info->failed = 0;
    info->skipped = 0;

    info->groupName = OC_STR8(name);
}

void oc_test_group_end(oc_test_info* info)
{
    if(info->verbosity >= OC_TEST_PRINT_GROUP_SUMMARY)
    {
        oc_test_status status = info->failed ? OC_TEST_FAIL : OC_TEST_PASS;

        printf("%s", oc_test_status_color_start[status]);
        printf("%s", oc_test_status_string[status]);
        printf("%s", oc_test_status_color_stop);

        printf(" %.*s: passed: %i, skipped: %i, failed: %i, total: %i\n",
               oc_str8_ip(info->groupName),
               info->passed,
               info->skipped,
               info->failed,
               info->passed + info->skipped + info->failed);
    }

    info->groupName = (oc_str8){};
}

#define oc_test_group(info, name) oc_defer_loop(oc_test_group_begin(info, name), oc_test_group_end(info))

void oc_test_init(oc_test_info* info, const char* name, oc_test_verbosity verbosity)
{
    info->name = OC_STR8(name);
    info->verbosity = verbosity;
}

void oc_test_summary(oc_test_info* info)
{
    if(info->verbosity >= OC_TEST_PRINT_ALL_FAILED)
    {
        printf("\n------------------------------------------------------------------------\n"
               "%.*s summary: passed: %i, skipped: %i, failed: %i, total: %i\n"
               "------------------------------------------------------------------------\n",
               oc_str8_ip(info->name),
               info->totalPassed,
               info->totalSkipped,
               info->totalFailed,
               info->totalPassed + info->totalSkipped + info->totalFailed);
    }
}

typedef struct test_subprocess_options
{
    oc_str8 exec;
    oc_str8 mode;
    oc_str8 command;

    i64 val1;
    i64 val2;

} test_subprocess_options;

oc_str8 CHILD_TEST_STRING = OC_STR8_LIT("Hello world");

int run_child(test_subprocess_options* options)
{
    if(!oc_str8_cmp(options->command, OC_STR8("add")))
    {
        return options->val1 + options->val2;
    }
    else if(!oc_str8_cmp(options->command, OC_STR8("print_test")))
    {
        printf("%.*s", oc_str8_ip(CHILD_TEST_STRING));
        return 0;
    }
    else
    {
        return -1;
    }
}

int run_tests(test_subprocess_options* options)
{
    oc_test_info info = { 0 };
    oc_test_init(&info, "subprocess", OC_TEST_PRINT_ALL);

    oc_test_group(&info, "oc_subprocess_spawn")
    {
        oc_test(&info, "pass arguments")
        {
            const char* args[] = {
                options->exec.ptr,
                "child",
                "add",
                "5",
                "2",
            };
            oc_subprocess subprocess = oc_catch(oc_subprocess_spawn(5, args, 0))
            {
                oc_test_fail(&info,
                             "oc_subprocess_spawn() error: %.*s",
                             oc_str8_ip(oc_subprocess_error_string(oc_last_error())));
            }
            else
            {
                oc_subprocess_completion comp = oc_catch(oc_subprocess_wait(subprocess))
                {
                    oc_test_fail(&info,
                                 "oc_subprocess_wait() error: %.*s",
                                 oc_str8_ip(oc_subprocess_error_string(oc_last_error())));
                }
                else if(comp.signal)
                {
                    oc_test_fail(&info, "child process terminated with signal %i", comp.signal);
                }
                else if(comp.returnCode != 7)
                {
                    oc_test_fail(&info, "child process returned wrong result %i (expected %i)", comp.returnCode, 7);
                }
                else
                {
                    oc_test_pass(&info);
                }
            }
        }

        oc_test(&info, "read")
        {
            const char* args[] = {
                options->exec.ptr,
                "child",
                "print_test"
            };
            oc_subprocess_spawn_options spawnOptions = {
                .stdOut = OC_SUBPROCESS_STDIO_PIPE,
            };
            oc_subprocess subprocess = oc_catch(oc_subprocess_spawn(3, args, &spawnOptions))
            {
                oc_test_fail(&info,
                             "oc_subprocess_spawn() error: %.*s",
                             oc_str8_ip(oc_subprocess_error_string(oc_last_error())));
            }
            else
            {
                oc_arena_scope scratch = oc_scratch_begin();

                oc_subprocess_completion comp = oc_catch(oc_subprocess_read_and_wait(scratch.arena, subprocess))
                {
                    oc_test_fail(&info,
                                 "oc_subprocess_read_and_wait() error: %.*s",
                                 oc_str8_ip(oc_subprocess_error_string(oc_last_error())));
                }
                else if(comp.signal)
                {
                    oc_test_fail(&info, "child process terminated with signal %i", comp.signal);
                }
                else if(comp.returnCode != 0)
                {
                    oc_test_fail(&info, "child process returned wrong result %i (expected %i)", comp.returnCode, 0);
                }
                else if(oc_str8_cmp(comp.capturedStdout, CHILD_TEST_STRING))
                {
                    oc_test_fail(&info,
                                 "captured output does not match test string. Received \"%.*s\" (expected \"%.*s\")",
                                 oc_str8_ip(comp.capturedStdout),
                                 oc_str8_ip(CHILD_TEST_STRING));
                }
                else
                {
                    oc_test_pass(&info);
                }
                oc_scratch_end(scratch);
            }
        }
    }

    oc_test_group(&info, "oc_subprocess_run")
    {
        oc_test(&info, "pass arguments")
        {
            const char* args[] = {
                options->exec.ptr,
                "child",
                "add",
                "5",
                "2",
            };
            oc_subprocess_completion comp = oc_catch(oc_subprocess_run(5, args, 0))
            {
                oc_test_fail(&info,
                             "oc_subprocess_run() error: %.*s",
                             oc_str8_ip(oc_subprocess_error_string(oc_last_error())));
            }
            else if(comp.signal)
            {
                oc_test_fail(&info, "child process terminated with signal %i", comp.signal);
            }
            else if(comp.returnCode != 7)
            {
                oc_test_fail(&info, "child process returned wrong result %i (expected %i)", comp.returnCode, 7);
            }
            else
            {
                oc_test_pass(&info);
            }
        }

        oc_test(&info, "read")
        {
            oc_arena_scope scratch = oc_scratch_begin();

            const char* args[] = {
                options->exec.ptr,
                "child",
                "print_test"
            };

            oc_subprocess_run_options runOptions = {
                .captureArena = scratch.arena,
                .stdOut = OC_SUBPROCESS_STDIO_PIPE,
            };

            oc_subprocess_completion comp = oc_catch(oc_subprocess_run(3, args, &runOptions))
            {
                oc_test_fail(&info,
                             "oc_subprocess_run() error: %.*s",
                             oc_str8_ip(oc_subprocess_error_string(oc_last_error())));
            }
            else if(comp.signal)
            {
                oc_test_fail(&info, "child process terminated with signal %i", comp.signal);
            }
            else if(comp.returnCode != 0)
            {
                oc_test_fail(&info, "child process returned wrong result %i (expected %i)", comp.returnCode, 0);
            }
            else if(oc_str8_cmp(comp.capturedStdout, CHILD_TEST_STRING))
            {
                oc_test_fail(&info,
                             "captured output does not match test string. Received \"%.*s\" (expected \"%.*s\")",
                             oc_str8_ip(comp.capturedStdout),
                             oc_str8_ip(CHILD_TEST_STRING));
            }
            else
            {
                oc_test_pass(&info);
            }

            oc_scratch_end(scratch);
        }
    }
    oc_test_summary(&info);
    return info.totalFailed ? -1 : 0;
}

int main(int argc, char** argv)
{
    oc_arena_scope scratch = oc_scratch_begin();

    test_subprocess_options options = {
        .exec = OC_STR8(argv[0]),
    };

    oc_arg_parser parser = { 0 };
    oc_arg_parser_init(&parser,
                       scratch.arena,
                       OC_STR8("test_subprocess"),
                       &(oc_arg_parser_options){
                           .desc = OC_STR8("Tests for Orca subprocess API."),
                       });

    oc_arg_parser* childParser = oc_arg_parser_subparser(&parser,
                                                         OC_STR8("child"),
                                                         &options.mode,
                                                         &(oc_arg_parser_options){
                                                             .desc = OC_STR8("Run as child process."),
                                                         });

    oc_arg_parser* addParser = oc_arg_parser_subparser(childParser,
                                                       OC_STR8("add"),
                                                       &options.command,
                                                       0);

    oc_arg_parser_add_positional_i64(addParser,
                                     OC_STR8("val1"),
                                     &options.val1,
                                     &(oc_arg_parser_arg_options){
                                         .required = true,
                                     });

    oc_arg_parser_add_positional_i64(addParser,
                                     OC_STR8("val2"),
                                     &options.val2,
                                     &(oc_arg_parser_arg_options){
                                         .required = true,
                                     });

    oc_arg_parser* printParser = oc_arg_parser_subparser(childParser,
                                                         OC_STR8("print_test"),
                                                         &options.command,
                                                         0);

    int result = 0;
    if(oc_arg_parser_parse(&parser, argc, argv))
    {
        result = -1;
    }
    else if(!oc_str8_cmp(options.mode, OC_STR8("child")))
    {
        result = run_child(&options);
    }
    else
    {
        result = run_tests(&options);
    }
    oc_scratch_end(scratch);
    return result;
}
