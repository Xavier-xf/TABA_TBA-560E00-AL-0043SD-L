#include "ui_api.h"
#include "os_sys_api.h"
#include "math.h"
#include "stdlib.h"

static int standby_timeout = 0;

static unsigned long long standby_timer_start = 0;

static void(*standby_timeout_func)(void) = NULL;

static bool standby_open_status = false;

bool standby_timer_open(int timeout,void(*timeout_callback)(void))
{
	if(timeout > 0)
	{
		standby_timeout = timeout;
	}

	if(timeout_callback != NULL)
	{
		standby_timeout_func = timeout_callback;
	}

	standby_timer_start = os_get_ms();
	
	standby_open_status = true;
	return true;
}


bool standby_timer_close(void)
{
	standby_open_status = false;
	return true;
}

bool standby_timer_check(void)
{
	if(standby_open_status == false)
	{
		return false;
	}

	unsigned long long standby_timer_end = os_get_ms();
	if(abs(standby_timer_end - standby_timer_start) > standby_timeout)
	{
		if(standby_timeout_func != NULL)
		{
			standby_timeout_func();
		}
		standby_timer_start = os_get_ms();		
		return true;
	}
	return false;
}

bool standby_timer_reset(void)
{
	if(standby_open_status == false)
	{
		return false;
	}
	
	standby_timer_start = os_get_ms();
	return true;
}




static bool timed_event_run_flag = false;
static void (*timed_event_check_func)(void) = NULL;
bool timed_event_check_init(void(*timed_check_func)(void))
{
    timed_event_run_flag = false;
    if(timed_check_func != NULL)
	{
        timed_event_check_func = timed_check_func;
        return true;
    }
    return false;
}
bool timed_event_check(void)
{
    if(timed_event_run_flag == false)
	{
        return false;
    }
	
    if(timed_event_check_func != NULL)
	{
        timed_event_check_func();
        return true;
    }
    return false;
}
void timed_event_check_start(void)
{
    timed_event_run_flag = true;
}

void timed_event_check_stop(void)
{
    timed_event_run_flag = false;
}



