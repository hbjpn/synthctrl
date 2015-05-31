#ifndef __UTIL__H__
#define __UTIL__H__

#include <queue>
#include <pthread.h>
#include <string>

#define AsByte(s) atoi((s).c_str())
#define AsChar(s) (s)[0]
#define AsInt(s) atoi((s).c_str())
#define AsInt64(s) atoll((s).c_str());

void AsByteArray(const char* s, std::vector<unsigned char>& data);

const char* token(const char* src, char* dst);

enum EventType{
	EVT_GPIO0_TRIGERRED = 0,
	EVT_GPIO1_TRIGERRED,
	EVT_DEPLOY,
    EVT_TIMER_EXPIRED,
    EVT_MIDI_IN,
	EVT_END_LOOP
};

struct Event{
	EventType _type;
	Event(EventType type) : _type(type){}; 
	virtual EventType type() const { return _type; }
};

struct EventDeploy : public Event
{
	std::string _fn;
	EventDeploy(const char* fn) : Event(EVT_DEPLOY){
		_fn = fn;
	}
};

struct EventMIDIIn : public Event
{
	std::vector<unsigned char> _data;
	EventMIDIIn(const std::vector<unsigned char>& data) : Event(EVT_MIDI_IN){
		_data = data;
	}
};

class EventQueue
{
	std::queue<const Event*> evt_queue;
	pthread_mutex_t evt_loop_mutex;
	pthread_cond_t evt_loop_cond;


public:
	EventQueue();

	virtual ~EventQueue();
	
	void push(const Event* evt);
	const Event* pop();
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

typedef void (*TimerCallback)(void);
int setTimer(long long int duration_ns, TimerCallback callback, int signo);


#endif
	
