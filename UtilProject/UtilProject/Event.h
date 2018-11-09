#ifndef _EVENT_H_
#define _EVENT_H_

#ifdef _WIN32
#include <Windows.h>
typedef HANDLE event_handle;
#else
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct{
    bool state;
    bool manual_reset;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}event_t;

typedef event_t* event_handle
#endif

//Return: If the function succeeds, the return value is a handle to the event object.
//If the function fails. the return value is NULL.
event_handle event_create(bool manual_reset, bool init_state);

//Return: If the event is waited success, the return value is zero
//or the return value is -1
int event_wait(event_handle hevent);

//Return£º0 wait succeed£¬1 timeout£¬-1 failed
int event_timedwait(event_handle hevent, long milliseconds);

//Return£º0 succeed£¬-1 falied
int event_set(event_handle hevent);

//Return£º0 succeed£¬-1 falied
int event_reset(event_handle hevent);

//Return£ºnone
void event_destroy(event_handle hevent);

#endif /*_EVENT_H_*/

