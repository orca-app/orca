#define OC_NO_APP_LAYER
#include "orca.c"
#include "util/tests.c"

int run_tests(oc_test_info* info)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_test_group(info, "oc_path_slice_directory")
    {
        oc_test(info, "/foo/bar")
        {
            oc_str8 res = oc_path_slice_directory(OC_STR8("/foo/bar"));
            if(oc_str8_cmp(res, OC_STR8("/foo")))
            {
                oc_test_fail(info, "%.*s (expected /foo)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo/bar/")
        {
            oc_str8 res = oc_path_slice_directory(OC_STR8("/foo/bar/"));
            if(oc_str8_cmp(res, OC_STR8("/foo/bar")))
            {
                oc_test_fail(info, "%.*s (expected /foo/bar)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo")
        {
            oc_str8 res = oc_path_slice_directory(OC_STR8("/foo"));
            if(oc_str8_cmp(res, OC_STR8("/")))
            {
                oc_test_fail(info, "%.*s (expected /)", oc_str8_ip(res));
            }
        }

        oc_test(info, "foo")
        {
            oc_str8 res = oc_path_slice_directory(OC_STR8("foo"));
            if(oc_str8_cmp(res, OC_STR8("")))
            {
                oc_test_fail(info, "%.*s (expected <empty>)", oc_str8_ip(res));
            }
        }

#if OC_PLATFORM_MACOS || OC_PLATFORM_LINUX
        oc_test(info, "\\foo\\bar")
        {
            oc_str8 res = oc_path_slice_directory(OC_STR8("\\foo\\bar"));
            if(oc_str8_cmp(res, OC_STR8("")))
            {
                oc_test_fail(info, "%.*s (expected <empty>)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/bar\\baz")
        {
            oc_str8 res = oc_path_slice_directory(OC_STR8("/bar\\baz"));
            if(oc_str8_cmp(res, OC_STR8("/")))
            {
                oc_test_fail(info, "%.*s (expected /)", oc_str8_ip(res));
            }
        }

#elif OC_PLATFORM_WINDOWS
        oc_test(info, "\\foo\\bar")
        {
            oc_str8 res = oc_path_slice_directory(OC_STR8("\\foo\\bar"));
            if(oc_str8_cmp(res, OC_STR8("\\foo")))
            {
                oc_test_fail(info, "%.*s (expected \\foo)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo\\bar")
        {
            oc_str8 res = oc_path_slice_directory(OC_STR8("/foo\\bar"));
            if(oc_str8_cmp(res, OC_STR8("/foo")))
            {
                oc_test_fail(info, "%.*s (expected /foo)", oc_str8_ip(res));
            }
        }

        oc_test(info, "C:\\foo\\bar")
        {
            oc_str8 res = oc_path_slice_directory(OC_STR8("C:\\foo\\bar"));
            if(oc_str8_cmp(res, OC_STR8("C:\\foo")))
            {
                oc_test_fail(info, "%.*s (expected C:\\foo)", oc_str8_ip(res));
            }
        }

        oc_test(info, "C:foo\\bar")
        {
            oc_str8 res = oc_path_slice_directory(OC_STR8("C:foo\\bar"));
            if(oc_str8_cmp(res, OC_STR8("C:foo")))
            {
                oc_test_fail(info, "%.*s (expected C:foo)", oc_str8_ip(res));
            }
        }

        oc_test(info, "C:foo")
        {
            oc_str8 res = oc_path_slice_directory(OC_STR8("C:foo"));
            if(oc_str8_cmp(res, OC_STR8("C:")))
            {
                oc_test_fail(info, "%.*s (expected C:)", oc_str8_ip(res));
            }
        }
#endif
    }

    oc_test_group(info, "oc_path_slice_filename")
    {
        oc_test(info, "/foo/bar")
        {
            oc_str8 res = oc_path_slice_filename(OC_STR8("/foo/bar"));
            if(oc_str8_cmp(res, OC_STR8("bar")))
            {
                oc_test_fail(info, "%.*s (expected bar)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo/bar.ext")
        {
            oc_str8 res = oc_path_slice_filename(OC_STR8("/foo/bar.ext"));
            if(oc_str8_cmp(res, OC_STR8("bar.ext")))
            {
                oc_test_fail(info, "%.*s (expected bar.ext)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo/bar/")
        {
            oc_str8 res = oc_path_slice_filename(OC_STR8("/foo/bar/"));
            if(oc_str8_cmp(res, OC_STR8("")))
            {
                oc_test_fail(info, "%.*s (expected <empty>)", oc_str8_ip(res));
            }
        }

        oc_test(info, "foo")
        {
            oc_str8 res = oc_path_slice_filename(OC_STR8("foo"));
            if(oc_str8_cmp(res, OC_STR8("foo")))
            {
                oc_test_fail(info, "%.*s (expected foo)", oc_str8_ip(res));
            }
        }

#if OC_PLATFORM_MACOS || OC_PLATFORM_LINUX
        oc_test(info, "foo\\bar")
        {
            oc_str8 res = oc_path_slice_filename(OC_STR8("foo\\bar"));
            if(oc_str8_cmp(res, OC_STR8("foo\\bar")))
            {
                oc_test_fail(info, "%.*s (expected foo\\bar)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo/bar\\baz")
        {
            oc_str8 res = oc_path_slice_filename(OC_STR8("/foo/bar\\baz"));
            if(oc_str8_cmp(res, OC_STR8("bar\\baz")))
            {
                oc_test_fail(info, "%.*s (expected bar\\baz)", oc_str8_ip(res));
            }
        }

#elif OC_PLATFORM_WINDOWS
        oc_test(info, "foo\\bar")
        {
            oc_str8 res = oc_path_slice_filename(OC_STR8("foo\\bar"));
            if(oc_str8_cmp(res, OC_STR8("bar")))
            {
                oc_test_fail(info, "%.*s (expected bar)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo/bar\\baz")
        {
            oc_str8 res = oc_path_slice_filename(OC_STR8("/foo/bar\\baz"));
            if(oc_str8_cmp(res, OC_STR8("baz")))
            {
                oc_test_fail(info, "%.*s (expected baz)", oc_str8_ip(res));
            }
        }

        oc_test(info, "C:\\foo")
        {
            oc_str8 res = oc_path_slice_filename(OC_STR8("C:\\foo"));
            if(oc_str8_cmp(res, OC_STR8("foo")))
            {
                oc_test_fail(info, "%.*s (expected foo)", oc_str8_ip(res));
            }
        }

        oc_test(info, "C:foo")
        {
            oc_str8 res = oc_path_slice_filename(OC_STR8("C:foo"));
            if(oc_str8_cmp(res, OC_STR8("foo")))
            {
                oc_test_fail(info, "%.*s (expected foo)", oc_str8_ip(res));
            }
        }
#endif
    }

    oc_test_group(info, "oc_path_slice_stem")
    {
        oc_test(info, "/foo/bar")
        {
            oc_str8 res = oc_path_slice_stem(OC_STR8("/foo/bar"));
            if(oc_str8_cmp(res, OC_STR8("bar")))
            {
                oc_test_fail(info, "%.*s (expected bar)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo/bar.ext")
        {
            oc_str8 res = oc_path_slice_stem(OC_STR8("/foo/bar.ext"));
            if(oc_str8_cmp(res, OC_STR8("bar")))
            {
                oc_test_fail(info, "%.*s (expected bar)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo/bar/.ext")
        {
            oc_str8 res = oc_path_slice_stem(OC_STR8("/foo/bar/.ext"));
            if(oc_str8_cmp(res, OC_STR8(".ext")))
            {
                oc_test_fail(info, "%.*s (expected .ext)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo/bar/.baz.ext")
        {
            oc_str8 res = oc_path_slice_stem(OC_STR8("/foo/bar/.baz.ext"));
            if(oc_str8_cmp(res, OC_STR8(".baz")))
            {
                oc_test_fail(info, "%.*s (.baz)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo/bar/..ext")
        {
            oc_str8 res = oc_path_slice_stem(OC_STR8("/foo/bar/..ext"));
            if(oc_str8_cmp(res, OC_STR8("..ext")))
            {
                oc_test_fail(info, "%.*s (expected ..ext)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo/bar/ext.")
        {
            oc_str8 res = oc_path_slice_stem(OC_STR8("/foo/bar/ext."));
            if(oc_str8_cmp(res, OC_STR8("ext")))
            {
                oc_test_fail(info, "%.*s (expected ext)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo/bar/ext..")
        {
            oc_str8 res = oc_path_slice_stem(OC_STR8("/foo/bar/ext.."));
            if(oc_str8_cmp(res, OC_STR8("ext.")))
            {
                oc_test_fail(info, "%.*s (expected ext.)", oc_str8_ip(res));
            }
        }

#if OC_PLATFORM_WINDOWS
        oc_test(info, "C:foo.ext")
        {
            oc_str8 res = oc_path_slice_stem(OC_STR8("C:foo.ext"));
            if(oc_str8_cmp(res, OC_STR8("foo")))
            {
                oc_test_fail(info, "%.*s (expected foo)", oc_str8_ip(res));
            }
        }
#endif
    }

    oc_test_group(info, "oc_path_slice_extension")
    {
        oc_test(info, "/foo/bar.ext")
        {
            oc_str8 res = oc_path_slice_extension(OC_STR8("/foo/bar.ext"));
            if(oc_str8_cmp(res, OC_STR8(".ext")))
            {
                oc_test_fail(info, "%.*s (expected .ext)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo/bar")
        {
            oc_str8 res = oc_path_slice_extension(OC_STR8("/foo/bar"));
            if(oc_str8_cmp(res, OC_STR8("")))
            {
                oc_test_fail(info, "%.*s (expected <empty>)", oc_str8_ip(res));
            }
        }

        oc_test(info, "/foo/bar.")
        {
            oc_str8 res = oc_path_slice_extension(OC_STR8("/foo/bar."));
            if(oc_str8_cmp(res, OC_STR8(".")))
            {
                oc_test_fail(info, "%.*s (expected .)", oc_str8_ip(res));
            }
        }
    }

    oc_test_group(info, "oc_path_split")
    {
        oc_test(info, "/foo/bar/baz.ext")
        {
            oc_str8_list list = oc_path_split(scratch.arena, OC_STR8("/foo/bar/baz.ext"));
            oc_str8 elements = oc_str8_list_collate(scratch.arena, list, OC_STR8("("), OC_STR8(","), OC_STR8(")"));

            if(list.list.count != 4)
            {
                oc_test_fail(info, "%.*s (expected (/,foo,bar,baz.ext))", oc_str8_ip(elements));
            }
            else
            {
                oc_str8_elt* elt0 = oc_list_first_entry(list.list, oc_str8_elt, listElt);
                oc_str8_elt* elt1 = oc_list_next_entry(elt0, oc_str8_elt, listElt);
                oc_str8_elt* elt2 = oc_list_next_entry(elt1, oc_str8_elt, listElt);
                oc_str8_elt* elt3 = oc_list_next_entry(elt2, oc_str8_elt, listElt);

                if(oc_str8_cmp(elt0->string, OC_STR8("/"))
                   || oc_str8_cmp(elt1->string, OC_STR8("foo"))
                   || oc_str8_cmp(elt2->string, OC_STR8("bar"))
                   || oc_str8_cmp(elt3->string, OC_STR8("baz.ext")))
                {
                    oc_test_fail(info, "%.*s (expected (/,foo,bar,baz.ext))", oc_str8_ip(elements));
                }
            }
        }

        oc_test(info, "//foo//bar//baz.ext")
        {
            oc_str8_list list = oc_path_split(scratch.arena, OC_STR8("//foo//bar//baz.ext"));
            oc_str8 elements = oc_str8_list_collate(scratch.arena, list, OC_STR8("("), OC_STR8(","), OC_STR8(")"));

            if(list.list.count != 4)
            {
                oc_test_fail(info, "%.*s (expected (/,foo,bar,baz.ext))", oc_str8_ip(elements));
            }
            else
            {
                oc_str8_elt* elt0 = oc_list_first_entry(list.list, oc_str8_elt, listElt);
                oc_str8_elt* elt1 = oc_list_next_entry(elt0, oc_str8_elt, listElt);
                oc_str8_elt* elt2 = oc_list_next_entry(elt1, oc_str8_elt, listElt);
                oc_str8_elt* elt3 = oc_list_next_entry(elt2, oc_str8_elt, listElt);

                if(oc_str8_cmp(elt0->string, OC_STR8("/"))
                   || oc_str8_cmp(elt1->string, OC_STR8("foo"))
                   || oc_str8_cmp(elt2->string, OC_STR8("bar"))
                   || oc_str8_cmp(elt3->string, OC_STR8("baz.ext")))
                {
                    oc_test_fail(info, "%.*s (expected (/,foo,bar,baz.ext))", oc_str8_ip(elements));
                }
            }
        }

#if OC_PLATFORM_WINDOWS
        oc_test(info, "C:\\foo\\bar\\baz.ext")
        {
            oc_str8_list list = oc_path_split(scratch.arena, OC_STR8("C:\\foo\\bar\\baz.ext"));
            oc_str8 elements = oc_str8_list_collate(scratch.arena, list, OC_STR8("("), OC_STR8(","), OC_STR8(")"));

            if(list.list.count != 4)
            {
                oc_test_fail(info, "%.*s (expected (C:\\,foo,bar,baz.ext))", oc_str8_ip(elements));
            }
            else
            {
                oc_str8_elt* elt0 = oc_list_first_entry(list.list, oc_str8_elt, listElt);
                oc_str8_elt* elt1 = oc_list_next_entry(elt0, oc_str8_elt, listElt);
                oc_str8_elt* elt2 = oc_list_next_entry(elt1, oc_str8_elt, listElt);
                oc_str8_elt* elt3 = oc_list_next_entry(elt2, oc_str8_elt, listElt);

                if(oc_str8_cmp(elt0->string, OC_STR8("C:\\"))
                   || oc_str8_cmp(elt1->string, OC_STR8("foo"))
                   || oc_str8_cmp(elt2->string, OC_STR8("bar"))
                   || oc_str8_cmp(elt3->string, OC_STR8("baz.ext")))
                {
                    oc_test_fail(info, "%.*s (expected (C:\\,foo,bar,baz.ext))", oc_str8_ip(elements));
                }
            }
        }

        oc_test(info, "C:foo\\bar\\baz.ext")
        {
            oc_str8_list list = oc_path_split(scratch.arena, OC_STR8("C:foo\\bar\\baz.ext"));
            oc_str8 elements = oc_str8_list_collate(scratch.arena, list, OC_STR8("("), OC_STR8(","), OC_STR8(")"));

            if(list.list.count != 4)
            {
                oc_test_fail(info, "%.*s (expected (C:,foo,bar,baz.ext))", oc_str8_ip(elements));
            }
            else
            {
                oc_str8_elt* elt0 = oc_list_first_entry(list.list, oc_str8_elt, listElt);
                oc_str8_elt* elt1 = oc_list_next_entry(elt0, oc_str8_elt, listElt);
                oc_str8_elt* elt2 = oc_list_next_entry(elt1, oc_str8_elt, listElt);
                oc_str8_elt* elt3 = oc_list_next_entry(elt2, oc_str8_elt, listElt);

                if(oc_str8_cmp(elt0->string, OC_STR8("C:"))
                   || oc_str8_cmp(elt1->string, OC_STR8("foo"))
                   || oc_str8_cmp(elt2->string, OC_STR8("bar"))
                   || oc_str8_cmp(elt3->string, OC_STR8("baz.ext")))
                {
                    oc_test_fail(info, "%.*s (expected (C:,foo,bar,baz.ext))", oc_str8_ip(elements));
                }
            }
        }
#endif
    }

    oc_test_group(info, "oc_path_join")
    {
        oc_test(info, "(/,foo,bar,baz.ext)")
        {
            oc_str8_list list = { 0 };
            oc_str8_list_push(scratch.arena, &list, OC_STR8("/"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("foo"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("bar"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("baz.ext"));

            oc_str8 path = oc_path_join(scratch.arena, list);
            if(oc_str8_cmp(path, OC_STR8("/foo/bar/baz.ext")))
            {
                oc_test_fail(info, "%.*s (expected /foo/bar/baz.ext)", oc_str8_ip(path));
            }
        }

        oc_test(info, "(/,foo,/,bar,baz.ext)")
        {
            oc_str8_list list = { 0 };
            oc_str8_list_push(scratch.arena, &list, OC_STR8("/"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("foo"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("/"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("bar"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("baz.ext"));

            oc_str8 path = oc_path_join(scratch.arena, list);
            if(oc_str8_cmp(path, OC_STR8("/foo/bar/baz.ext")))
            {
                oc_test_fail(info, "%.*s (expected /foo/bar/baz.ext)", oc_str8_ip(path));
            }
        }

        oc_test(info, "(/,foo,<empty>,bar,baz.ext)")
        {
            oc_str8_list list = { 0 };
            oc_str8_list_push(scratch.arena, &list, OC_STR8("/"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("foo"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8(""));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("bar"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("baz.ext"));

            oc_str8 path = oc_path_join(scratch.arena, list);
            if(oc_str8_cmp(path, OC_STR8("/foo/bar/baz.ext")))
            {
                oc_test_fail(info, "%.*s (expected /foo/bar/baz.ext)", oc_str8_ip(path));
            }
        }

        oc_test(info, "(/,foo,bar,baz,/)")
        {
            oc_str8_list list = { 0 };
            oc_str8_list_push(scratch.arena, &list, OC_STR8("/"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("foo"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("bar"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("baz"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("/"));

            oc_str8 path = oc_path_join(scratch.arena, list);
            if(oc_str8_cmp(path, OC_STR8("/foo/bar/baz/")))
            {
                oc_test_fail(info, "%.*s (expected /foo/bar/baz/)", oc_str8_ip(path));
            }
        }

        oc_test(info, "(/,foo,/bar,baz.ext)")
        {
            oc_str8_list list = { 0 };
            oc_str8_list_push(scratch.arena, &list, OC_STR8("/"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("foo"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("/bar"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("baz.ext"));

            oc_str8 path = oc_path_join(scratch.arena, list);
            if(oc_str8_cmp(path, OC_STR8("/foo/bar/baz.ext")))
            {
                oc_test_fail(info, "%.*s (expected /foo/bar/baz.ext)", oc_str8_ip(path));
            }
        }

        oc_test(info, "(/,foo,bar/,baz.ext)")
        {
            oc_str8_list list = { 0 };
            oc_str8_list_push(scratch.arena, &list, OC_STR8("/"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("foo"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("bar/"));
            oc_str8_list_push(scratch.arena, &list, OC_STR8("baz.ext"));

            oc_str8 path = oc_path_join(scratch.arena, list);
            if(oc_str8_cmp(path, OC_STR8("/foo/bar/baz.ext")))
            {
                oc_test_fail(info, "%.*s (expected /foo/bar/baz.ext)", oc_str8_ip(path));
            }
        }
    }

    oc_test_group(info, "oc_path_append")
    {
        oc_test(info, "/foo, bar")
        {
            oc_str8 path = oc_path_append(scratch.arena, OC_STR8("/foo"), OC_STR8("bar"));
            if(oc_str8_cmp(path, OC_STR8("/foo/bar")))
            {
                oc_test_fail(info, "%.*s (expected /foo/bar)", oc_str8_ip(path));
            }
        }
    }

    oc_scratch_end(scratch);

    oc_test_summary(info);
    return info->totalFailed ? -1 : 0;
}

int main()
{
    oc_test_info info = { 0 };
    oc_test_init(&info, "path", OC_TEST_PRINT_ALL);

    return run_tests(&info);
}
