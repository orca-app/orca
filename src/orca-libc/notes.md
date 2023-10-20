
To Review:

- `exit`, `assert`

Should be inculded by missing or incomplete:

- `malloc` --> replace with our modified `dlmalloc`
- in `stdio.h`: `scanf` family
- in `stdlib.h`: `atof`, `strtol` family

Missing or incomplete, maybe included later:

- `stdatomic.h` (when we have atomics)
- `threads.h` (when we have threading)
- `time.h` (except things depending on locale or setting time)

Excluded APIs:

- `locale.h`
- `setjmp.h`
- `signal.h`
- File IO, `wchar`, `uchar`