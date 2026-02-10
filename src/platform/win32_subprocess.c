#include <unistd.h>
#include <errno.h>

#include "subprocess.h"

typedef struct oc_subprocess_info
{
    HANDLE processHandle;

    //pipes
    HANDLE stdInHandle;
    HANDLE stdOutHandle;
    HANDLE stdErrHandle;
    //...
} oc_subprocess_info;

oc_subprocess_error oc_win32_subprocess_last_error()
{
    //TODO
    return OC_SUBPROCESS_UNKNOWN;
}

oc_subprocess_spawn_result oc_subprocess_spawn(int argc, char** argv, oc_subprocess_spawn_options* optionsPtr)
{
    //TODO: it's pretty hard to detect if we pass the wrong argc here...
    oc_subprocess_error error = OC_SUBPROCESS_OK;
    oc_subprocess_spawn_options options = optionsPtr ? *optionsPtr : (oc_subprocess_spawn_options){ 0 };

    //NOTE: index 0 is the read end, 1 is the write end
    HANDLE childStdIn[2] = { 0 };
    HANDLE childStdOut[2] = { 0 };
    HANDLE childStdErr[2] = { 0 };

    SECURITY_ATTRIBUTES attr = {
        .nLength = sizeof(SECURITY_ATTRIBUTES),
        .bInheritHandle = TRUE,
    };

    if(options.stdOut == OC_SUBPROCESS_STDIO_PIPE)
    {
        if(!CreatePipe(&childStdOut[0], &childStdOut[1], &attr, 0))
        {
            oc_subprocess_error error = oc_win32_subprocess_last_error();
            return oc_result_error(oc_subprocess_spawn_result, error);
        }
        if(!SetHandleInformation(childStdOut[0], HANDLE_FLAG_INHERIT, 0))
        {
            oc_subprocess_error error = oc_win32_subprocess_last_error();
            return oc_result_error(oc_subprocess_spawn_result, error);
        }
    }
    else
    {
        childStdOut[1] = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    if(options.stdErr == OC_SUBPROCESS_STDIO_PIPE)
    {
        if(!CreatePipe(&childStdErr[0], &childStdErr[1], &attr, 0))
        {
            oc_subprocess_error error = oc_win32_subprocess_last_error();
            return oc_result_error(oc_subprocess_spawn_result, error);
        }
        if(!SetHandleInformation(childStdErr[1], HANDLE_FLAG_INHERIT, 0))
        {
            oc_subprocess_error error = oc_win32_subprocess_last_error();
            return oc_result_error(oc_subprocess_spawn_result, error);
        }
    }
    else
    {
        childStdErr[1] = GetStdHandle(STD_ERROR_HANDLE);
    }

    if(options.stdIn == OC_SUBPROCESS_STDIO_PIPE)
    {
        if(!CreatePipe(&childStdIn[0], &childStdIn[1], &attr, 0))
        {
            oc_subprocess_error error = oc_win32_subprocess_last_error();
            return oc_result_error(oc_subprocess_spawn_result, error);
        }
        if(!SetHandleInformation(childStdIn[1], HANDLE_FLAG_INHERIT, 0))
        {
            oc_subprocess_error error = oc_win32_subprocess_last_error();
            return oc_result_error(oc_subprocess_spawn_result, error);
        }
    }
    else
    {
        childStdIn[0] = GetStdHandle(STD_INPUT_HANDLE);
    }

    oc_arena_scope scratch = oc_scratch_begin();

    oc_str16 execName = oc_win32_utf8_to_wide(scratch.arena, OC_STR8(argv[0]));
    oc_str16 commandLine = { 0 };
    {
        oc_str8_list list = { 0 };

        for(int i = 0; i < argc; i++)
        {
            oc_str8_list_pushf(scratch.arena, &list, "\"%.s\"");
            if(i < argc - 1)
            {
                oc_str8_list_push(scratch.arena, &list, OC_STR8(" "));
            }
        }
        oc_str8 commandLineU8 = oc_str8_list_join(scratch.arena, list);
        commandLine = oc_win32_utf8_to_wide(scratch.arena, commandLineU8);
    }

    STARTUPINFOW startupInfo = {
        .cb = sizeof(STARTUPINFO),
        .dwFlags = STARTF_USESTDHANDLES,
        .hStdInput = childStdIn[0],
        .hStdOutput = childStdOut[1],
        .hStdError = childStdErr[1],
    };
    PROCESS_INFORMATION processInfo = { 0 };

    BOOL r = CreateProcessW(execName.ptr,
                            commandLine.ptr,
                            NULL,
                            NULL,
                            TRUE,
                            0,
                            NULL,
                            NULL,
                            &startupInfo,
                            &processInfo);

    oc_scratch_end(scratch);

    if(r == FALSE)
    {
        return oc_result_error(oc_subprocess_spawn_result, oc_win32_subprocess_last_error());
    }
    else
    {
        oc_subprocess_info* info = oc_malloc_type(oc_subprocess_info);
        info->processHandle = processInfo.hProcess;

        if(options.stdIn == OC_SUBPROCESS_STDIO_PIPE)
        {
            info->stdInHandle = childStdIn[1];
            CloseHandle(childStdIn[0]);
        }
        if(options.stdOut == OC_SUBPROCESS_STDIO_PIPE)
        {
            info->stdOutHandle = childStdOut[0];
            CloseHandle(childStdOut[1]);
        }
        if(options.stdErr == OC_SUBPROCESS_STDIO_PIPE)
        {
            info->stdErrHandle = childStdErr[0];
            CloseHandle(childStdErr[1]);
        }

        CloseHandle(processInfo.hThread);

        return oc_result_value(oc_subprocess_spawn_result, info);
    }
}

oc_subprocess_result oc_subprocess_read_and_wait(oc_arena* arena, oc_subprocess subprocess)
{
    /*
    oc_subprocess_result result = { 0 };
    oc_subprocess_completion completion = { 0 };

    //NOTE: drain subprocess's output streams. If we just wait without doing this, we could be in a
    // deadlock situation where the child process has filled its output buffers and is blocked on a
    // write call.

    {
        oc_arena_scope scratch = arena ? oc_scratch_begin_next(arena) : oc_scratch_begin();

        oc_str8_list outList = { 0 };
        oc_str8_list errList = { 0 };
        ssize_t nOut = 0;
        ssize_t nErr = 0;
        char chunk[1024];

        //NOTE: here we set the read end of the subprocess pipe to non blocking and alternatively read
        // from the child process's stdout and stderr, until everything is collected. This avoids
        // blocking on reading one fd while the child process is blocked writing on the other one.
        //TODO: check errors
        fcntl(subprocess->stdOutFd, F_SETFL, O_NONBLOCK);
        fcntl(subprocess->stdErrFd, F_SETFL, O_NONBLOCK);

        while(1)
        {
            if(subprocess->stdOutFd >= 0)
            {
                nOut = read(subprocess->stdOutFd, chunk, 1024);
                if(nOut == -1)
                {
                    if(errno != EAGAIN)
                    {
                        break;
                    }
                }
                else if(nOut)
                {
                    oc_str8_list_push(scratch.arena, &outList, oc_str8_from_buffer(nOut, chunk));
                }
            }

            if(subprocess->stdErrFd >= 0)
            {
                nErr = read(subprocess->stdErrFd, chunk, 1024);
                if(nErr == -1)
                {
                    if(errno != EAGAIN)
                    {
                        break;
                    }
                }
                else if(nErr)
                {
                    oc_str8_list_push(scratch.arena, &errList, oc_str8_from_buffer(nErr, chunk));
                }
            }

            if(nOut == 0 && nErr == 0)
            {
                break;
            }
        }

        if(nOut == -1 || nErr == -1)
        {
            //TODO convert errno
            //TODO: should we close fds/wait/free anyway?
            return oc_result_error(oc_subprocess_result, OC_SUBPROCESS_UNKNOWN);
        }
        else
        {
            if(arena)
            {
                completion.capturedStdout = oc_str8_list_join(arena, outList);
                completion.capturedStderr = oc_str8_list_join(arena, errList);
            }
        }
        oc_scratch_end(scratch);
    }

    int stat = 0;
    if(waitpid(subprocess->pid, &stat, 0) == subprocess->pid)
    {
        if(WIFEXITED(stat))
        {
            completion.returnCode = (int)(signed char)WEXITSTATUS(stat);
        }
        if(WIFSIGNALED(stat))
        {
            completion.signal = WTERMSIG(stat);
        }

        result = oc_result_value(oc_subprocess_result, completion);
    }
    else
    {
        oc_subprocess_error err = 0;
        switch(errno)
        {
            case ECHILD:
                err = OC_SUBPROCESS_NO_CHILD;
                break;

            case EINTR:
                err = OC_SUBPROCESS_INTERRUPTED;
                break;

            default:
                err = OC_SUBPROCESS_UNKNOWN;
        }
        result = oc_result_error(oc_subprocess_result, err);
    }

end:
    if(subprocess->stdInFd >= 0)
    {
        close(subprocess->stdInFd);
    }
    if(subprocess->stdOutFd >= 0)
    {
        close(subprocess->stdOutFd);
    }
    if(subprocess->stdErrFd >= 0)
    {
        close(subprocess->stdErrFd);
    }
    free(subprocess);
    return result;
    */
    return oc_result_error(oc_subprocess_result, OC_SUBPROCESS_UNKNOWN);
}

oc_subprocess_result oc_subprocess_wait(oc_subprocess subprocess)
{
    return oc_subprocess_read_and_wait(0, subprocess);
}

oc_subprocess_result oc_subprocess_run(int argc, char** argv, oc_subprocess_run_options* options)
{
    /*
    oc_subprocess_run_options runOption = options ? *options : (oc_subprocess_run_options){ 0 };

    oc_subprocess_spawn_options spawnOptions = {};
    oc_subprocess_spawn_result spawn = oc_subprocess_spawn(argc, argv, &spawnOptions);

    oc_subprocess subprocess = oc_catch(spawn)
    {
        return oc_result_error(oc_subprocess_result, spawn.error);
    }

    return oc_subprocess_read_and_wait(runOption.captureArena, subprocess);
    */
    //TODO
    return oc_result_error(oc_subprocess_result, OC_SUBPROCESS_UNKNOWN);
}
