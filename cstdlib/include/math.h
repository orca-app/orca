// NOTE(orca): not doing anything fancy for float_t and double_t
typedef float float_t;
typedef double double_t;

#define NAN       __builtin_nanf("")
#define INFINITY  __builtin_inff()

double      fabs(double);

double      pow(double, double);
