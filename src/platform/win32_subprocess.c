/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "platform_subprocess.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

oc_str8 oc_run_cmd(oc_arena* arena, oc_str8 cmd)
{
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 out = { 0 };

    HANDLE childStdinRd = NULL;
    HANDLE childStdinWr = NULL;
    HANDLE childStdoutRd = NULL;
    HANDLE childStdoutWr = NULL;

    // Create child process's stdin and stdout pipes
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.lpSecurityDescriptor = NULL;
    saAttr.bInheritHandle = TRUE;

    if(!CreatePipe(&childStdinRd, &childStdinWr,
                   &saAttr, 0)
       || !SetHandleInformation(childStdinWr, HANDLE_FLAG_INHERIT, 0))
    {
        oc_log_error("failed to create child stdin\n");
        goto error;
    }
    if(!CreatePipe(&childStdoutRd, &childStdoutWr,
                   &saAttr, 0)
       || !SetHandleInformation(childStdoutRd, HANDLE_FLAG_INHERIT, 0))
    {
        oc_log_error("failed to create child stdout\n");
        goto error;
    }

    // Create child process
    char* cmdStr = oc_str8_to_cstring(scratch.arena, cmd);
    PROCESS_INFORMATION procInfo;
    STARTUPINFO startInfo;

    memset(&procInfo, 0, sizeof(PROCESS_INFORMATION));
    memset(&startInfo, 0, sizeof(STARTUPINFO));

    startInfo.cb = sizeof(STARTUPINFO);
    startInfo.hStdInput = childStdinRd;
    startInfo.hStdOutput = childStdoutWr;
    startInfo.hStdError = childStdoutWr;
    startInfo.dwFlags |= STARTF_USESTDHANDLES;

    BOOL success = CreateProcess(
        NULL,
        cmdStr,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &startInfo,
        &procInfo);
    if(!success)
    {
        oc_log_error("failed to create child process\n");
        goto error;
    }
    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);
    CloseHandle(childStdinRd); // the parent does not need these ends of the pipes
    CloseHandle(childStdoutWr);

    // TODO: Optionally write to the child's stdin
    if(!CloseHandle(childStdinWr))
    {
        oc_log_error("failed to close child's stdin");
        goto error;
    }

    // Read child's combined stdout/stderr
    u64 outputLen = 0;
    char* outputBuf = NULL;
    while(true)
    {
// TODO: apply smartness to this number
#define CHUNK_SIZE 1024
        char chunkBuf[CHUNK_SIZE];
        DWORD nRead;
        BOOL success = ReadFile(childStdoutRd, chunkBuf, CHUNK_SIZE, &nRead, NULL);
        if(!success || nRead == 0)
        {
            break;
        }

        char* dst = oc_arena_push(arena, nRead);
        if(!outputBuf)
        {
            outputBuf = dst;
        }
        memcpy(dst, chunkBuf, nRead);
        outputLen += nRead;
    }

    out = oc_str8_from_buffer(outputLen, outputBuf);
    CloseHandle(childStdinWr);
    CloseHandle(childStdoutRd);
    goto cleanup;

error:
    LPVOID msgBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&msgBuf,
        0, NULL);
    oc_log_error("%s\n", msgBuf);

    LocalFree(msgBuf);
cleanup:
    oc_scratch_end(scratch);

    return out;
}
