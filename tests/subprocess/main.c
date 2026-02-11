#define OC_NO_APP_LAYER 1
#include "orca.c"
#include "util/tests.c"

typedef struct test_subprocess_options
{
    oc_str8 exec;
    oc_str8 mode;
    oc_str8 command;

    f64 delay;
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
        if(options->delay)
        {
            oc_sleep_nano(options->delay * 1e9);
        }
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

        oc_test(&info, "read delayed")
        {
            // This checks that oc_subprocess_read_and_wait reads until the write end has closed
            const char* args[] = {
                options->exec.ptr,
                "child",
                "print_test",
                "--delay",
                "0.5"
            };
            oc_subprocess_spawn_options spawnOptions = {
                .stdOut = OC_SUBPROCESS_STDIO_PIPE,
            };
            oc_subprocess subprocess = oc_catch(oc_subprocess_spawn(5, args, &spawnOptions))
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

    oc_arg_parser_add_named_f64(printParser, OC_STR8("delay"), &options.delay, 0);

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
