#include "util.h"
#include <stdio.h>

EventQueue::EventQueue ()
{
    pthread_mutex_init (&evt_loop_mutex, NULL);
    pthread_cond_init (&evt_loop_cond, NULL);
};

EventQueue::~EventQueue ()
{
    pthread_mutex_destroy (&evt_loop_mutex);
    pthread_cond_destroy (&evt_loop_cond);
}

void
EventQueue::push (Event evt)
{
    pthread_mutex_lock (&evt_loop_mutex);
    evt_queue.push (evt);
    pthread_cond_signal (&evt_loop_cond);
    pthread_mutex_unlock (&evt_loop_mutex);
}

Event
EventQueue::pop ()
{
    pthread_mutex_lock (&evt_loop_mutex);
    while (evt_queue.empty ()) {
	pthread_cond_wait (&evt_loop_cond, &evt_loop_mutex);
    }
    Event evt = evt_queue.front ();
    evt_queue.pop ();
    pthread_mutex_unlock (&evt_loop_mutex);
    return evt;
}

Thread::Thread ()
{
    pthread_create (&thread, NULL, Thread::runHelper, (void*)this);
}

void
Thread::join ()
{
    pthread_join (thread, NULL);
}

void*
Thread::runHelper (void *This)
{
    ((Thread *) This)->run ();
}

Thread::~Thread ()
{
}

