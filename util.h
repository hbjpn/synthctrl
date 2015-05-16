#ifndef __UTIL__H__
#define __UTIL__H__

#include <queue>
#include <pthread.h>

#define AsByte(s) atoi((s).c_str())
#define AsChar(s) (s)[0]

enum EventType{
	EVT_GPIO0_TRIGERRED = 0,
	EVT_GPIO1_TRIGERRED,
	EVT_END_LOOP
};

struct Event{
	EventType type;
};

class EventQueue
{
	std::queue<Event> evt_queue;
	pthread_mutex_t evt_loop_mutex;
	pthread_cond_t evt_loop_cond;


public:
	EventQueue();

	virtual ~EventQueue();
	
	void push(Event evt);
	Event pop();
};

class Thread{
	pthread_t thread;
public:
	Thread();
	void join();
	static void* runHelper(void* This);
	virtual ~Thread();
	virtual void run() = 0;
};


#endif
	
