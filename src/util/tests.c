/*************************************************************************
*
*  Orca
*  Copyright 2026 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "util/typedefs.h"
#include "util/macros.h"
#include "util/memory.h"
#include "util/strings.h"

typedef enum oc_test_status
{
    OC_TEST_FAIL = 0,
    OC_TEST_SKIP,
    OC_TEST_PASS,
    OC_TEST_INFO,
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
    "[INFO]",
};

static const char* oc_test_status_color_start[] = {
    "\033[38;5;9m\033[1m",
    "\033[38;5;13m\033[1m",
    "\033[38;5;10m\033[1m",
    "\033[38;5;14m\033[1m",
};

static const char* oc_test_status_color_stop = "\033[m";

void oc_test_mark_fmt(oc_test_info* info, oc_test_status status, const char* fmt, ...)
{
    OC_ASSERT(status == OC_TEST_INFO || info->testName.len, "this should be called within a oc_test_begin/end block.");
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
        default:
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
    if(!info->statusSet)
    {
        oc_test_pass(info);
    }
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

    printf("%s", oc_test_status_color_start[OC_TEST_INFO]);
    printf("%s", oc_test_status_string[OC_TEST_INFO]);
    printf("%s", oc_test_status_color_stop);
    printf(" testing %s...\n", name);
}

void oc_test_group_end(oc_test_info* info)
{
    if(info->verbosity >= OC_TEST_PRINT_GROUP_SUMMARY)
    {
        oc_test_status status = info->failed ? OC_TEST_FAIL : OC_TEST_PASS;

        printf("%s", oc_test_status_color_start[status]);
        printf("%s", oc_test_status_string[status]);
        printf("%s", oc_test_status_color_stop);

        printf(" %.*s tests: passed: %i, skipped: %i, failed: %i, total: %i\n",
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

    printf("----------------------------------------------------------------------------\n"
           " Start test: %s\n"
           "----------------------------------------------------------------------------\n",
           name);
}

void oc_test_summary(oc_test_info* info)
{
    if(info->verbosity >= OC_TEST_PRINT_ALL_FAILED)
    {
        printf("----------------------------------------------------------------------------\n"
               " %.*s test summary: passed: %i, skipped: %i, failed: %i, total: %i\n"
               "----------------------------------------------------------------------------\n\n",
               oc_str8_ip(info->name),
               info->totalPassed,
               info->totalSkipped,
               info->totalFailed,
               info->totalPassed + info->totalSkipped + info->totalFailed);
    }
}
