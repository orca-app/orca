
#define OC_NO_APP_LAYER 1
#include "orca.c"
#include "util/tests.c"

typedef oc_option_type(i32) oc_option_i32;
typedef oc_ptr_option_type(i32) oc_ptr_option_i32;

void test_options(oc_test_info* info)
{
    oc_test_group(info, "valid value option")
    {
        oc_option_i32 optI32 = oc_option_value(oc_option_i32, 42);

        oc_test(info, "check")
        {
            if(!oc_option_check(optI32))
            {
                oc_test_fail(info, "failed to check valid option.");
            }
        }

        oc_test(info, "unwrap")
        {
            if(oc_option_unwrap(optI32) != 42)
            {
                oc_test_fail(info, "failed to unwrap valid option.");
            }
        }

        oc_test(info, "if")
        {
            i32 val = oc_option_if(optI32)
            {
                if(val != 42)
                {
                    oc_test_fail(info, "failed to get value from oc_option_if().");
                }
            }
            else
            {
                oc_test_fail(info, "oc_option_if() failed for valid option.");
            }
        }

        oc_test(info, "orelse")
        {
            i32 val = oc_option_orelse(optI32)
            {
                oc_test_fail(info, "oc_option_orelse() caught valid option.");
            }
            if(!info->statusSet)
            {
                if(val != 42)
                {
                    oc_test_fail(info, "oc_option_oresle() returned zero value for valid option.");
                }
            }
        }
    }

    oc_test_group(info, "nil value option")
    {
        oc_option_i32 optNil = oc_option_nil(oc_option_i32);

        oc_test(info, "check")
        {
            if(oc_option_check(optNil))
            {
                oc_test_fail(info, "invalid option passed check.");
            }
        }

        oc_test(info, "if")
        {
            i32 val = oc_option_if(optNil)
            {
                oc_test_fail(info, "oc_option_if() failed for invalid option.");
            }
            else if(val != 0)
            {
                oc_test_fail(info, "oc_option_if() returned non-null value for invalid option.");
            }
        }

        oc_test(info, "orelse")
        {
            bool caught = false;
            i32 val = oc_option_orelse(optNil)
            {
                caught = true;
                if(val != 0)
                {
                    oc_test_fail(info, "oc_option_orelse() returned non-null value for invalid option.");
                }
            }
            if(!caught)
            {
                oc_test_fail(info, "oc_option_orelse() did not caught invalid option.");
            }
        }
    }

    oc_test_group(info, "valid pointer option")
    {
        i32 val = 42;
        oc_ptr_option_i32 ptrOptI32 = oc_option_ptr(oc_ptr_option_i32, &val);

        oc_test(info, "check")
        {
            if(!oc_option_check(ptrOptI32))
            {
                oc_test_fail(info, "failed to check valid pointer option.");
            }
        }

        oc_test(info, "unwrap")
        {
            if(!oc_option_unwrap(ptrOptI32) || *oc_option_unwrap(ptrOptI32) != 42)
            {
                oc_test_fail(info, "failed to unwrap valid pointer option.");
            }
        }

        oc_test(info, "if")
        {
            i32* p = oc_option_if(ptrOptI32)
            {
                if(!p || *p != 42)
                {
                    oc_test_fail(info, "failed to get pointer from oc_option_if().");
                }
            }
            else
            {
                oc_test_fail(info, "oc_option_if() failed for valid pointer option.");
            }
        }

        oc_test(info, "orelse")
        {
            i32* p = oc_option_orelse(ptrOptI32)
            {
                oc_test_fail(info, "oc_option_orelse() caught valid pointer option.");
            }
            if(!info->statusSet && (!p || *p != 42))
            {
                oc_test_fail(info, "oc_option_oresle() returned null pointer for valid pointer option.");
            }
        }
    }

    oc_test_group(info, "nil pointer option")
    {
        oc_ptr_option_i32 ptrOptNil = oc_option_nil(oc_ptr_option_i32);

        oc_test(info, "check")
        {
            if(oc_option_check(ptrOptNil))
            {
                oc_test_fail(info, "nil pointer option passed check.");
            }
        }

        oc_test(info, "if")
        {
            i32* p = oc_option_if(ptrOptNil)
            {
                oc_test_fail(info, "oc_option_if() failed for nil pointer option.");
            }
            else if(p != 0)
            {
                oc_test_fail(info, "oc_option_if() returned non-null pointer for nil pointer option.");
            }
        }

        oc_test(info, "orelse")
        {
            bool caught = false;
            i32* p = oc_option_orelse(ptrOptNil)
            {
                caught = true;
                if(p != 0)
                {
                    oc_test_fail(info, "oc_option_orelse() returned non-null pointer for nil pointer option.");
                }
            }
            if(!caught)
            {
                oc_test_fail(info, "oc_option_orelse() did not caught nil pointer option.");
            }
        }
    }
}

typedef oc_result_type(i32, i32) oc_result_i32;

oc_result_i32 test_try_value()
{
    oc_result_i32 test = oc_result_value(oc_result_i32, 35);
    i32 val = oc_try(test);

    return oc_result_value(oc_result_i32, 42);
}

oc_result_i32 test_try_error()
{
    oc_result_i32 test = oc_result_error(oc_result_i32, -1);
    i32 val = oc_try(test);

    return oc_result_error(oc_result_i32, -2);
}

void test_results(oc_test_info* info)
{
    oc_test_group(info, "valid result")
    {
        oc_result_i32 resVal = oc_result_value(oc_result_i32, 42);

        oc_test(info, "check")
        {
            if(!oc_result_check(resVal))
            {
                oc_test_fail(info, "oc_result_check() failed on valid result.");
            }
        }

        oc_test(info, "unwrap")
        {
            if(oc_result_unwrap(resVal) != 42)
            {
                oc_test_fail(info, "failed to unwrap valid result.");
            }
        }

        oc_test(info, "if")
        {
            i32 val = oc_result_if(resVal)
            {
                if(val != 42)
                {
                    oc_test_fail(info, "Get wrong value from oc_result_if().");
                }
            }
            else
            {
                oc_test_fail(info, "oc_result_if() failed on valid result.");
            }
        }

        oc_test(info, "catch")
        {
            i32 val = oc_catch(resVal)
            {
                oc_test_fail(info, "oc_catch() caught valid result.");
            }
            else if(val != 42)
            {
                oc_test_fail(info, "oc_catch() return wrong value for valid result.");
            }
        }

        oc_test(info, "try")
        {
            oc_result_i32 resTryVal = test_try_value();
            if(resTryVal.error)
            {
                oc_test_fail(info, "oc_try() returned an error on valid result.");
            }
            else if(resTryVal.value != 42)
            {
                oc_test_fail(info, "oc_try() returned with value on valid result.");
            }
        }
    }

    oc_test_group(info, "invalid result")
    {
        oc_result_i32 resErr = oc_result_error(oc_result_i32, -1);

        oc_test(info, "check")
        {
            if(oc_result_check(resErr))
            {
                oc_test_fail(info, "oc_result_check() passed on error result.");
            }
        }

        oc_test(info, "if")
        {
            i32 val = oc_result_if(resErr)
            {
                oc_test_fail(info, "oc_result_if() passed on error result.");
            }
            else if(val != 0)
            {
                oc_test_fail(info, "oc_result_if() returned non-null value error result.");
            }
            else if(oc_last_error() != -1)
            {
                oc_test_fail(info, "oc_last_error() returned wrong error after failed oc_result_if().");
            }
        }

        oc_test(info, "catch")
        {
            bool caught = false;
            i32 val = oc_catch(resErr)
            {
                caught = true;
                if(oc_last_error() != -1)
                {
                    oc_test_fail(info, "oc_last_error() returned wrong error in oc_catch() block.");
                }
            }
            if(!caught)
            {
                oc_test_fail(info, "oc_catch() did not caught error result.");
            }
            else if(val != 0)
            {
                oc_test_fail(info, "oc_catch() return non-null value for error result.");
            }
        }

        oc_test(info, "try")
        {
            oc_result_i32 resTryErr = test_try_error();
            if(resTryErr.error == 0)
            {
                oc_test_fail(info, "oc_try() returned a value on error.");
            }
            else if(resTryErr.error == -2)
            {
                oc_test_fail(info, "oc_try() did not return on error.");
            }
            else if(resTryErr.error != -1)
            {
                oc_test_fail(info, "oc_try() returned wrong error.");
            }
        }
    }
}

int main()
{
    oc_test_info info = { 0 };
    oc_test_init(&info, "wrapped_types", OC_TEST_PRINT_ALL);

    test_options(&info);
    test_results(&info);

    oc_test_summary(&info);
    return info.totalFailed ? -1 : 0;
}
