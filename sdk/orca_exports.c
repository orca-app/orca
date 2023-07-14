#include"orca.h"

ORCA_EXPORT mp_event* _OrcaGetRawEventPtr()
{
	static mp_event event;
	return &event;
}

ORCA_EXPORT void _OrcaClockInit()
{
	mp_clock_init();
}