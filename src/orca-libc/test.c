#include <math.h>
#include <stdlib.h>
#include <stdio.h>

__attribute__((export_name("main"))) int main()
{
    //    x = cos(x);

    //    int l = strtol("123", 0, 10);

    return (0);
}

__attribute__((export_name("foo"))) double foo(double a)
{
    return (cos(a));
}

__attribute__((export_name("bar"))) double bar(const char* s)
{

    double res = 0;
    sscanf(s, "%lf", &res);
    return (res);
}

__attribute__((export_name("baz"))) char* baz(char* buff)
{
    sprintf(buff, "Hello, %f\n", 3.14);
    return (buff);
}
