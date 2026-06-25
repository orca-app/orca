#define OC_NO_APP_LAYER 1
#include "orca.c"
#include "util/tests.c"

int main(int argc, char** argv)
{
    oc_test_info test = { 0 };
    oc_test_init(&test, "heap", OC_TEST_PRINT_ALL);

    oc_heap heap = { 0 };
    oc_heap_init(&heap);

    oc_test(&test, "init")
    {
        if(oc_heap_debug_check_consistency(&heap))
        {
            oc_test_fail(&test, "failed heap consistency check");
        }
    }

    oc_test_group(&test, "small alloc and free")
    {
        char* p = 0;
        oc_test(&test, "alloc")
        {
            p = oc_heap_alloc(&heap, 24);
            if(!p)
            {
                oc_test_fail(&test, "failed to allocate");
            }
            else
            {
                memset(p, 0xfe, 24);

                if(oc_heap_debug_check_consistency(&heap))
                {
                    oc_test_fail(&test, "failed heap consistency check");
                }
                else if(!oc_heap_debug_is_allocated(&heap, p, 24))
                {
                    oc_test_fail(&test, "can't check allocation");
                }
            }
        }

        oc_test(&test, "free")
        {
            oc_heap_free(&heap, p);
            if(oc_heap_debug_check_consistency(&heap))
            {
                oc_test_fail(&test, "failed heap consistency check");
            }
            else if(oc_heap_debug_is_allocated(&heap, p, 24))
            {
                oc_test_fail(&test, "p still allocated after free");
            }
        }
    }

    oc_test_group(&test, "large alloc and free")
    {
        char* p = 0;
        oc_test(&test, "alloc")
        {
            p = oc_heap_alloc(&heap, 4000);

            if(!p)
            {
                oc_test_fail(&test, "failed to allocate");
            }
            else
            {
                memset(p, 0xfe, 4000);

                if(oc_heap_debug_check_consistency(&heap))
                {
                    oc_test_fail(&test, "failed heap consistency check");
                }
                else if(!oc_heap_debug_is_allocated(&heap, p, 24))
                {
                    oc_test_fail(&test, "can't check allocation");
                }
                else if(oc_typed_list_count(heap.regions) != 2)
                {
                    oc_test_fail(&test, "region count should now be 2 (got %llu)", oc_typed_list_count(heap.regions));
                }
            }
        }

        oc_test(&test, "free")
        {
            oc_heap_free(&heap, p);
            if(oc_heap_debug_check_consistency(&heap))
            {
                oc_test_fail(&test, "failed heap consistency check");
            }
            else if(oc_heap_debug_is_allocated(&heap, p, 24))
            {
                oc_test_fail(&test, "p still allocated after free");
            }
        }
    }

    oc_heap_cleanup(&heap);

    oc_test_summary(&test);
    return test.totalFailed ? -1 : 0;
}
