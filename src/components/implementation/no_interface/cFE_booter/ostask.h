#ifndef OSTASK_H
#define OSTASK_H

#include <sl.h>

#define HZ_PAUSE_US (1000 * 1000) //1sec

/* We delegate the main thread of execution to a different thread
 * (the main thread needs to run the scheduling loop)
 */
#define MAIN_DELEGATE_THREAD_PRIORITY 2
#define SENSOREMU_THREAD_PRIORITY (MAIN_DELEGATE_THREAD_PRIORITY + 1)
#define TIMER_THREAD_PRIORITY (MAIN_DELEGATE_THREAD_PRIORITY + 1)

/*
 * ThreadId overrides for apps
 */
extern thdid_t id_overrides[SL_MAX_NUM_THDS];

void OS_SchedulerStart(cos_thd_fn_t main_delegate);

#endif