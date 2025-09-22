#include <unistd.h>
#include <errno.h>

#include "subprocess.h"

typedef struct oc_subprocess_info
{
    pid_t pid;
    int stdInFd;
    int stdOutFd;
    int stdErrFd;
    //...
} oc_subprocess_info;

oc_subprocess_error oc_subprocess_create_pipes(int fildes[2])
{
    oc_subprocess_error error = OC_SUBPROCESS_OK;

    int r = pipe(fildes);
    if(r)
    {
        switch(errno)
        {
            case EMFILE:
            case ENFILE:
                error = OC_SUBPROCESS_PIPE;
                break;
            default:
                error = OC_SUBPROCESS_UNKNOWN;
                break;
        }
    }
    return error;
}

oc_subprocess_spawn_result oc_subprocess_spawn(int argc, char** argv, oc_subprocess_spawn_options* options)
{
    //TODO: it's pretty hard to detect if we pass the wrong argc here...

    int childStdIn[2] = { 0 };
    int childStdOut[2] = { 0 };
    int childStdErr[2] = { 0 };

    oc_subprocess_error error = oc_subprocess_create_pipes(childStdIn);
    if(error != OC_SUBPROCESS_OK)
    {
        return oc_wrap_error(oc_subprocess_spawn_result, error);
    }

    error = oc_subprocess_create_pipes(childStdOut);
    if(error != OC_SUBPROCESS_OK)
    {
        return oc_wrap_error(oc_subprocess_spawn_result, error);
    }

    error = oc_subprocess_create_pipes(childStdErr);
    if(error != OC_SUBPROCESS_OK)
    {
        return oc_wrap_error(oc_subprocess_spawn_result, error);
    }

    pid_t pid = fork();
    if(pid == 0)
    {
        //NOTE: child process

        //TODO: modify file descriptors if needed

        //NOTE: connect stdout to write end of pipe
        close(childStdIn[1]);
        close(childStdOut[0]);
        close(childStdErr[0]);

        dup2(childStdIn[0], 0);
        dup2(childStdOut[1], 1);
        dup2(childStdErr[1], 2);

        oc_arena_scope scratch = oc_scratch_begin();
        char** terminatedArgv = oc_arena_push_array(scratch.arena, char*, argc + 1);
        memcpy(terminatedArgv, argv, argc * sizeof(char*));

        //TODO: modify environ if needed
        int r = execvp(argv[0], terminatedArgv);

        //NOTE: if we returned from execvp there was an error
        //TODO: we should actually check these _before_ execve, because we can't return here...
        printf("couldn't execvp: %i\n", errno);
        exit(-1);
        /*
        oc_subprocess_error err = 0;
        switch(errno)
        {
            case EACCES:
            case ENOEXEC:
                err = OC_SUBPROCESS_NOT_EXECUTABLE;
                break;
            case ENOENT:
                err = OC_SUBPROCESS_NOT_FOUND;
                break;
            default:
                err = OC_SUBPROCESS_UNKNOWN;
                break;
        }
        return oc_wrap_error(oc_subprocess_spawn_result, err);
        */
    }
    else
    {
        close(childStdIn[0]);
        close(childStdOut[1]);
        close(childStdErr[1]);

        oc_subprocess_info* info = oc_malloc_type(oc_subprocess_info);
        info->pid = pid;
        info->stdInFd = childStdIn[1];
        info->stdOutFd = childStdOut[0];
        info->stdErrFd = childStdErr[0];

        //NOTE: parent process
        return oc_wrap_value(oc_subprocess_spawn_result, info);
    }
}

typedef oc_result(oc_str8, oc_subprocess_error) oc_subprocess_read_result;

oc_subprocess_read_result oc_subprocess_read_fd(oc_arena* arena, int fd)
{
    oc_str8_list list = { 0 };
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    ssize_t n = 0;
    char chunk[1024];

    while(1)
    {
        n = read(fd, chunk, 1024);
        if(n == -1)
        {
            break;
        }
        else if(n == 0)
        {
            break;
        }
        else
        {
            oc_str8_list_push(scratch.arena, &list, oc_str8_from_buffer(n, chunk));
        }
    }

    oc_subprocess_read_result result = { 0 };
    if(n)
    {
        //TODO convert errno
        result = oc_wrap_error(oc_subprocess_read_result, OC_SUBPROCESS_UNKNOWN);
    }
    else
    {
        oc_str8 s = oc_str8_list_join(arena, list);
        result = oc_wrap_value(oc_subprocess_read_result, s);
    }

    oc_scratch_end(scratch);
    return result;
}

oc_subprocess_result oc_subprocess_read_and_wait(oc_arena* arena, oc_subprocess subprocess)
{
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

            if(nOut == 0 && nErr == 0)
            {
                break;
            }
        }

        if(nOut == -1 || nErr == -1)
        {
            //TODO convert errno
            //TODO: should we close fds/wait/free anyway?
            return oc_wrap_error(oc_subprocess_result, OC_SUBPROCESS_UNKNOWN);
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

        result = oc_wrap_value(oc_subprocess_result, completion);
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
        result = oc_wrap_error(oc_subprocess_result, err);
    }

end:
    close(subprocess->stdInFd);
    close(subprocess->stdOutFd);
    close(subprocess->stdErrFd);
    free(subprocess);
    return result;
}

oc_subprocess_result oc_subprocess_wait(oc_subprocess subprocess)
{
    return oc_subprocess_read_and_wait(0, subprocess);
}

oc_subprocess_result oc_subprocess_run(int argc, char** argv, oc_subprocess_run_options* options)
{
    oc_subprocess_run_options runOption = options ? *options : (oc_subprocess_run_options){ 0 };

    oc_subprocess_spawn_options spawnOptions = {};
    oc_subprocess_spawn_result spawn = oc_subprocess_spawn(argc, argv, &spawnOptions);

    oc_subprocess subprocess = oc_catch(spawn)
    {
        return oc_wrap_error(oc_subprocess_result, spawn.error);
    }

    return oc_subprocess_read_and_wait(runOption.captureArena, subprocess);
}
