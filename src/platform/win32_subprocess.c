#include <unistd.h>
#include <errno.h>

#include "platform/subprocess.c"

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

    oc_arena_scope scratch = oc_scratch_begin();

    oc_str16 execName = oc_win32_utf8_to_wide(scratch.arena, OC_STR8(argv[0]));
    oc_str16 commandLine = { 0 };
    {
        oc_str8_list list = { 0 };

        for(int i = 0; i < argc; i++)
        {
            oc_str8_list_pushf(scratch.arena, &list, "\"%s\"", argv[i]);
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
        memset(info, 0, sizeof(oc_subprocess_info));

        info->processHandle = processInfo.hProcess;
        info->stdInHandle = childStdIn[1];
        info->stdOutHandle = childStdOut[0];
        info->stdErrHandle = childStdErr[0];

        if(options.stdIn == OC_SUBPROCESS_STDIO_PIPE)
        {
            CloseHandle(childStdIn[0]);
        }
        if(options.stdOut == OC_SUBPROCESS_STDIO_PIPE)
        {
            CloseHandle(childStdOut[1]);
        }
        if(options.stdErr == OC_SUBPROCESS_STDIO_PIPE)
        {
            CloseHandle(childStdErr[1]);
        }

        CloseHandle(processInfo.hThread);

        return oc_result_value(oc_subprocess_spawn_result, info);
    }
}

oc_subprocess_result oc_subprocess_read_and_wait(oc_arena* arena, oc_subprocess subprocess)
{
    oc_subprocess_completion completion = { 0 };
    oc_subprocess_error error = OC_SUBPROCESS_OK;

    //NOTE: drain subprocess's output streams. If we just wait without doing this, we could be in a
    // deadlock situation where the child process has filled its output buffers and is blocked on a
    // write call.
    {
        //NOTE: here we set the read end of the subprocess pipe to non blocking and alternatively read
        // from the child process's stdout and stderr, until everything is collected. This avoids
        // blocking on reading one fd while the child process is blocked writing on the other one.
        //TODO: check errors
        DWORD mode = PIPE_NOWAIT;
        SetNamedPipeHandleState(subprocess->stdOutHandle,
                                &mode,
                                NULL,
                                NULL);

        SetNamedPipeHandleState(subprocess->stdErrHandle,
                                &mode,
                                NULL,
                                NULL);

        oc_arena_scope scratch = arena ? oc_scratch_begin_next(arena) : oc_scratch_begin();

        oc_str8_list outList = { 0 };
        oc_str8_list errList = { 0 };
        DWORD nOut = 0;
        DWORD nErr = 0;
        char chunk[1024];

        bool stdOutClosed = subprocess->stdOutHandle ? false : true;
        bool stdErrClosed = subprocess->stdErrHandle ? false : true;

        while(!stdOutClosed || !stdErrClosed)
        {
            if(!stdOutClosed)
            {
                BOOL r = ReadFile(subprocess->stdOutHandle, chunk, 1024, &nOut, NULL);
                if(r == FALSE)
                {
                    DWORD lastErr = GetLastError();
                    if(lastErr == ERROR_BROKEN_PIPE)
                    {
                        stdOutClosed = true;
                    }
                    else if(lastErr != ERROR_NO_DATA)
                    {
                        error = oc_win32_subprocess_last_error();
                        break;
                    }
                }
                else if(nOut)
                {
                    oc_str8_list_push(scratch.arena, &outList, oc_str8_from_buffer(nOut, chunk));
                }
            }

            if(!stdErrClosed)
            {
                BOOL r = ReadFile(subprocess->stdErrHandle, chunk, 1024, &nErr, NULL);
                if(r == FALSE)
                {
                    DWORD lastErr = GetLastError();
                    if(lastErr == ERROR_BROKEN_PIPE)
                    {
                        stdErrClosed = true;
                    }
                    else if(lastErr != ERROR_NO_DATA)
                    {
                        error = oc_win32_subprocess_last_error();
                        break;
                    }
                }
                else if(nErr)
                {
                    oc_str8_list_push(scratch.arena, &errList, oc_str8_from_buffer(nErr, chunk));
                }
            }
        }

        if(error == OC_SUBPROCESS_OK && arena)
        {
            completion.capturedStdout = oc_str8_list_join(arena, outList);
            completion.capturedStderr = oc_str8_list_join(arena, errList);
        }
        oc_scratch_end(scratch);
    }

    //NOTE: if there's an error trying to wait the process would block
    // indefinitely
    if(error == OC_SUBPROCESS_OK)
    {
        DWORD r = WaitForSingleObject(subprocess->processHandle, INFINITE);
        if(r == WAIT_OBJECT_0)
        {
            //TODO
            DWORD exitCode;
            if(!GetExitCodeProcess(subprocess->processHandle, &exitCode))
            {
                error = oc_win32_subprocess_last_error();
            }
            else
            {
                if((i32)exitCode >= -255 && (i32)exitCode <= 255)
                {
                    completion.returnCode = exitCode;
                }
                else
                {
                    completion.signal = exitCode;
                }
            }
        }
        else if(r == WAIT_FAILED)
        {
            error = oc_win32_subprocess_last_error();
        }
        else
        {
            //NOTE: other possible return values of WaitForSingleObject() shouldn't make
            // sense for our pipe
            error = OC_SUBPROCESS_UNKNOWN;
        }
    }

    //NOTE: we still close the handles and free the subprocess even if there was an error
    if(subprocess->stdInHandle)
    {
        CloseHandle(subprocess->stdInHandle);
    }
    if(subprocess->stdOutHandle)
    {
        CloseHandle(subprocess->stdOutHandle);
    }
    if(subprocess->stdErrHandle)
    {
        CloseHandle(subprocess->stdErrHandle);
    }
    CloseHandle(subprocess->processHandle);

    free(subprocess);

    if(error != OC_SUBPROCESS_OK)
    {
        return oc_result_error(oc_subprocess_result, error);
    }
    else
    {
        return oc_result_value(oc_subprocess_result, completion);
    }
}
