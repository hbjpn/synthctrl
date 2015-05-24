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
EventQueue::push (const Event* evt)
{
    pthread_mutex_lock (&evt_loop_mutex);
    evt_queue.push (evt);
    pthread_cond_signal (&evt_loop_cond);
    pthread_mutex_unlock (&evt_loop_mutex);
}

const Event*
EventQueue::pop ()
{
    pthread_mutex_lock (&evt_loop_mutex);
    while (evt_queue.empty ()) {
	pthread_cond_wait (&evt_loop_cond, &evt_loop_mutex);
    }
    const Event* evt = evt_queue.front ();
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





/*
 *  sample program
 *  POSIX interval timer 
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

void SignalHandler(int signum, siginfo_t *info, void *ctxt)
{
    void* userdata = info->si_value.sival_ptr;
    if(userdata)
        ((TimerCallback)userdata)();
}

int setTimer(long long int duration_ns, TimerCallback callback, int signo)
{
    printf("duration_ns = %lld\n", duration_ns);
	struct sigaction action;
	struct sigevent	evp;
	struct itimerspec ispec;
	timer_t timerid = 0;

	memset(&action, 0, sizeof(action));
	memset(&evp, 0, sizeof(evp));

	/* set signal handler */
	action.sa_sigaction = SignalHandler;
	action.sa_flags = SA_SIGINFO | SA_RESTART;
	sigemptyset(&action.sa_mask);
	if(sigaction(SIGRTMIN + 1, &action, NULL) < 0){
		perror("sigaction error");
		exit(1);
	}

	/* set intarval timer (10ms) */
	evp.sigev_notify = SIGEV_SIGNAL;
	evp.sigev_signo = signo;
    evp.sigev_value.sival_ptr = (void*)(callback);
	if(timer_create(CLOCK_REALTIME, &evp, &timerid) < 0){
		perror("timer_create error");
		exit(1);
	}
	ispec.it_interval.tv_sec = 0;
	ispec.it_interval.tv_nsec = 0; // 10000000; One shot timer
	ispec.it_value.tv_sec  = int(duration_ns / 1000000000LL);
	ispec.it_value.tv_nsec = int(duration_ns % 1000000000LL);
	if(timer_settime(timerid, 0, &ispec, NULL) < 0){
		perror("timer_settime error");
		exit(1);
	}

    return 0;
}

