
#define OC_NO_APP_LAYER 1
#include "orca.c"

typedef oc_option_type(i32) oc_option_i32;
typedef oc_ptr_option_type(i32) oc_ptr_option_i32;

int test_options()
{
    //--------------------------------------------
    oc_log_info("Testing valid value option...\n");

    oc_option_i32 optI32 = oc_option_value(oc_option_i32, 42);
    if(!oc_option_check(optI32))
    {
        oc_log_error("failed to check valid option.\n");
        return -1;
    }

    if(oc_option_unwrap(optI32) != 42)
    {
        oc_log_error("failed to unwrap valid option.\n");
        return -1;
    }

    i32 val = oc_option_if(optI32)
    {
        if(val != 42)
        {
            oc_log_error("failed to get value from oc_option_if().\n");
            return -1;
        }
    }
    else
    {
        oc_log_error("oc_option_if() failed for valid option.\n");
        return -1;
    }

    val = oc_option_orelse(optI32)
    {
        oc_log_error("oc_option_orelse() caught valid option.\n");
        return -1;
    }
    if(val != 42)
    {
        oc_log_error("oc_option_oresle() returned zero value for valid option.\n");
        return -1;
    }

    //--------------------------------------------
    oc_log_info("Testing nil value option...\n");

    oc_option_i32 optNil = oc_option_nil(oc_option_i32);
    if(oc_option_check(optNil))
    {
        oc_log_error("invalid option passed check.\n");
        return -1;
    }

    val = oc_option_if(optNil)
    {
        oc_log_error("oc_option_if() failed for invalid option.\n");
        return -1;
    }
    else if(val != 0)
    {
        oc_log_error("oc_option_if() returned non-null value for invalid option.\n");
        return -1;
    }

    bool caught = false;
    val = oc_option_orelse(optNil)
    {
        caught = true;
        if(val != 0)
        {
            oc_log_error("oc_option_orelse() returned non-null value for invalid option.\n");
            return -1;
        }
    }
    if(!caught)
    {
        oc_log_error("oc_option_orelse() did not caught invalid option.\n");
        return -1;
    }

    //--------------------------------------------
    oc_log_info("Testing valid pointer option...\n");

    val = 42;
    oc_ptr_option_i32 ptrOptI32 = oc_option_ptr(oc_ptr_option_i32, &val);
    if(!oc_option_check(ptrOptI32))
    {
        oc_log_error("failed to check valid pointer option.\n");
        return -1;
    }

    if(!oc_option_unwrap(ptrOptI32) || *oc_option_unwrap(ptrOptI32) != 42)
    {
        oc_log_error("failed to unwrap valid pointer option.\n");
        return -1;
    }

    i32* p = oc_option_if(ptrOptI32)
    {
        if(!p || *p != 42)
        {
            oc_log_error("failed to get pointer from oc_option_if().\n");
            return -1;
        }
    }
    else
    {
        oc_log_error("oc_option_if() failed for valid pointer option.\n");
        return -1;
    }

    p = oc_option_orelse(ptrOptI32)
    {
        oc_log_error("oc_option_orelse() caught valid pointer option.\n");
        return -1;
    }
    if(!p || *p != 42)
    {
        oc_log_error("oc_option_oresle() returned null pointer for valid pointer option.\n");
        return -1;
    }

    //--------------------------------------------
    oc_log_info("Testing nil pointer option...\n");

    oc_ptr_option_i32 ptrOptNil = oc_option_nil(oc_ptr_option_i32);
    if(oc_option_check(ptrOptNil))
    {
        oc_log_error("nil pointer option passed check.\n");
        return -1;
    }

    p = oc_option_if(ptrOptNil)
    {
        oc_log_error("oc_option_if() failed for nil pointer option.\n");
        return -1;
    }
    else if(p != 0)
    {
        oc_log_error("oc_option_if() returned non-null pointer for nil pointer option.\n");
        return -1;
    }

    caught = false;
    p = oc_option_orelse(ptrOptNil)
    {
        caught = true;
        if(p != 0)
        {
            oc_log_error("oc_option_orelse() returned non-null pointer for nil pointer option.\n");
            return -1;
        }
    }
    if(!caught)
    {
        oc_log_error("oc_option_orelse() did not caught nil pointer option.\n");
        return -1;
    }

    return 0;
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

int test_results()
{
    //--------------------------------------------
    oc_log_info("Testing valid result...\n");

    oc_result_i32 resVal = oc_result_value(oc_result_i32, 42);
    if(!oc_result_check(resVal))
    {
        oc_log_error("oc_result_check() failed on valid result.\n");
        return -1;
    }
    if(oc_result_unwrap(resVal) != 42)
    {
        oc_log_error("failed to unwrap valid result.\n");
        return -1;
    }
    i32 val = oc_result_if(resVal)
    {
        if(val != 42)
        {
            oc_log_error("Get wrong value from oc_result_if().\n");
            return -1;
        }
    }
    else
    {
        oc_log_error("oc_result_if() failed on valid result.\n");
        return -1;
    }
    val = oc_catch(resVal)
    {
        oc_log_error("oc_catch() caught valid result.\n");
        return -1;
    }
    if(val != 42)
    {
        oc_log_error("oc_catch() return wrong value for valid result.\n");
        return -1;
    }
    oc_result_i32 resTryVal = test_try_value();
    if(resTryVal.error)
    {
        oc_log_error("oc_try() returned an error on valid result.\n");
        return -1;
    }
    else if(resTryVal.value != 42)
    {
        oc_log_error("oc_try() returned with value on valid result.\n");
        return -1;
    }

    //--------------------------------------------
    oc_log_info("Testing invalid result...\n");

    oc_result_i32 resErr = oc_result_error(oc_result_i32, -1);
    if(oc_result_check(resErr))
    {
        oc_log_error("oc_result_check() passed on error result.\n");
        return -1;
    }

    val = oc_result_if(resErr)
    {
        oc_log_error("oc_result_if() passed on error result.\n");
        return -1;
    }
    else if(val != 0)
    {
        oc_log_error("oc_result_if() returned non-null value error result.\n");
        return -1;
    }
    else if(oc_last_error() != -1)
    {
        oc_log_error("oc_last_error() returned wrong error after failed oc_result_if().\n");
        return -1;
    }

    bool caught = false;
    val = oc_catch(resErr)
    {
        caught = true;
        if(oc_last_error() != -1)
        {
            oc_log_error("oc_last_error() returned wrong error in oc_catch() block.\n");
            return -1;
        }
    }
    if(!caught)
    {
        oc_log_error("oc_catch() did not caught error result.\n");
        return -1;
    }
    else if(val != 0)
    {
        oc_log_error("oc_catch() return non-null value for error result.\n");
        return -1;
    }

    oc_result_i32 resTryErr = test_try_error();
    if(resTryErr.error == 0)
    {
        oc_log_error("oc_try() returned a value on error.\n");
        return -1;
    }
    else if(resTryErr.error == -2)
    {
        oc_log_error("oc_try() did not return on error.\n");
    }
    else if(resTryErr.error != -1)
    {
        oc_log_error("oc_try() returned wrong error.\n");
        return -1;
    }

    return 0;
}

int main()
{
    printf("\n");

    if(test_options())
    {
        return -1;
    }
    if(test_results())
    {
        return -1;
    }

    printf("\n");
    return 0;
}
