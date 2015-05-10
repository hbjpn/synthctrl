//#include <alsa/asoundlib.h>     /* Interface to the ALSA system */

#include <unistd.h>             /* for sleep() function */
#include <pthread.h>

#include <queue>
#include <signal.h>

#include "RtMidi.h"
#include "picojson.h"

#include "generic.h"
#include "korg.h"

#include "io.h"
#ifdef LINUX
#include "raspi_gpio.h"
#endif

///////////////////////////////////////////////////////////////////////////
void dump(unsigned char* buf, int len)
{
	for(int i = 0; i < len; ++i){
		fprintf(stderr, "0x%02x ", buf[i]);
	}
	fprintf(stderr, "\n");
}

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
	EventQueue(){
		pthread_mutex_init(&evt_loop_mutex, NULL);
		pthread_cond_init(&evt_loop_cond, NULL);
	};

	virtual ~EventQueue(){
		pthread_mutex_destroy(&evt_loop_mutex);
		pthread_cond_destroy(&evt_loop_cond);
	}
	
	void push(Event evt){
		pthread_mutex_lock(&evt_loop_mutex);
		evt_queue.push(evt);
		pthread_cond_signal(&evt_loop_cond);
		pthread_mutex_unlock(&evt_loop_mutex);
	}
	Event pop(){
		pthread_mutex_lock(&evt_loop_mutex);
		while(evt_queue.empty()){
			pthread_cond_wait(&evt_loop_cond, &evt_loop_mutex);
		}
		Event evt = evt_queue.front();
		evt_queue.pop();
		pthread_mutex_unlock(&evt_loop_mutex);
		return evt;
	}
};

EventQueue* evt_queue = NULL;

typedef int (*midigen)(unsigned char*, std::map<std::string, std::string>&);
void* event_loop(void* arg)
{
	evt_queue = new EventQueue();
	while(1){
		Event evt = evt_queue->pop();

		/* process event */
		printf("Event triggered : %d\n", evt.type);

		if(evt.type == EVT_END_LOOP)
			break;
	}
	fprintf(stderr, "End event loop thread\n");
	delete evt_queue;
	evt_queue = NULL;
	return NULL;
}

void gpio_port0_triggered()
{
	Event evt;
	evt.type = EVT_GPIO0_TRIGERRED;
	evt_queue->push(evt);
}

void gpio_port1_triggered()
{
	Event evt;
	evt.type = EVT_GPIO1_TRIGERRED;
	evt_queue->push(evt);
}


bool wait_signal()
{

  sigset_t ss;
  int ret, signo;

  sigemptyset(&ss);
  ret = sigaddset(&ss, SIGHUP);
  if (ret != 0) return false;
  ret = sigaddset(&ss, SIGTERM);
  if (ret != 0) return false;
  ret = sigaddset(&ss, SIGINT);
  if (ret != 0) return false;
  ret = sigprocmask(SIG_BLOCK, &ss, NULL);
  if (ret != 0) 
    return false;

  while(1) {
    if (sigwait(&ss, &signo) == 0) {
      /* handle SIGHUP */
      Event evt;
      evt.type = EVT_END_LOOP;
      evt_queue->push(evt);
      break;
    }
  }

  return true;
}


void start()
{
	pthread_t threads[1];
	pthread_create(&threads[0], NULL, event_loop, NULL);

	wait_signal(); // Ctrl+C is monitored
	fprintf(stderr, "Waiting for end threads ... \n");
	pthread_join(threads[0],NULL);
}

int main(int argc, char *argv[]) {
	const char* portname = "hw:1,0,0";  // see alsarawportlist.c example program
	/*if ((argc > 1) && (strncmp("hw:", argv[1], 3) == 0)) {
		portname = argv[1];
	}*/

	std::map<std::string, midigen> mgm;
	mgm["noteon"]     = generic::noteon;
	mgm["noteoff"]    = generic::noteoff;
	mgm["korg::zone"] = korg::zone;
	mgm["korg::tone"] = korg::tone;
	mgm["korg::solo"] = korg::solo;
	mgm["korg::mute"] = korg::mute;

	RtMidiOut* midiOut = new RtMidiOut();
	unsigned int nports = midiOut->getPortCount();
	std::cerr << nports << " MIDI ports available." << std::endl;
	midiOut->openPort(0);
	
	unsigned char midimsg[128];
	int len;

	config cfg;
	loadConfig(argv[1], cfg);

#ifdef LINUX
	RaspiGPIO* gpio = new RaspiGPIO();
	gpio->setCallback(0, gpio_port0_triggered);
	gpio->setCallback(1, gpio_port1_triggered);
	start();
#else
	for(int i = 0; i < cfg.size(); ++i){
		std::string& type = cfg[i]["type"];
		len = mgm[type](midimsg, cfg[i]);
		dump(midimsg, len);
		std::vector<unsigned char> msg(&midimsg[0], &midimsg[len]);
		midiOut->sendMessage(&msg);
	}  
#endif

#ifdef LINUX
	delete gpio;
#endif

	delete midiOut;
	return 0;          // so might be a good idea to erase it after closing.
}

///////////////////////////////////////////////////////////////////////////

