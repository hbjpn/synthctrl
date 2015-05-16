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

// Global variables
RtMidiOut* midiOut = NULL;

///////////////////////////////////////////////////////////////////////////
void dump(unsigned char* buf, int len)
{
	for(int i = 0; i < len; ++i){
		fprintf(stderr, "0x%02x ", buf[i]);
	}
	fprintf(stderr, "\n");
}

class MIDICtrlSet
{
	std::vector< std::vector<unsigned char> > _midiMsgs;
public:
	MIDICtrlSet(){};
	void add(const std::vector<unsigned char>& msg){
		_midiMsgs.push_back(msg);
	}

	void add(const unsigned char* msg, int len){
		std::vector<unsigned char> _tmp( &msg[0], &msg[len] );
		add(_tmp);
	}

	void run(RtMidiOut* midiOut)
	{
		for(int i = 0; i < _midiMsgs.size(); ++i){
			midiOut->sendMessage(&_midiMsgs[i]);
			dump(&_midiMsgs[i][0], _midiMsgs[i].size());
		}
	}
};

EventQueue* evt_queue = NULL;
MIDICtrlSet mcs;

typedef int (*midigen)(unsigned char*, std::map<std::string, std::string>&);
void* event_loop(void* arg)
{
	evt_queue = new EventQueue();
	bool loop_continue = true;
	while(loop_continue){
		Event evt = evt_queue->pop();

		/* process event */
		printf("Event triggered : %d\n", evt.type);

		switch(evt.type){
		case EVT_GPIO0_TRIGERRED:
		case EVT_GPIO1_TRIGERRED:
			mcs.run(midiOut);
			break;
		case EVT_END_LOOP:
			loop_continue = false;
			break;
		default:
			break;
		}
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

	std::map<std::string, midigen> mgm;
	mgm["noteon"]     = generic::noteon;
	mgm["noteoff"]    = generic::noteoff;
	mgm["korg::zone"] = korg::zone;
	mgm["korg::tone"] = korg::tone;
	mgm["korg::solo"] = korg::solo;
	mgm["korg::mute"] = korg::mute;

	midiOut = new RtMidiOut();
	unsigned int nports = midiOut->getPortCount();
	std::cerr << nports << " MIDI ports available. Choose number : " << std::endl;
	int port = 0;
	std::cin >> port;
	midiOut->openPort(port);
	
	unsigned char midimsg[128];
	int len;

	config cfg;
	loadConfig(argv[1], cfg);

	for(int i = 0; i < cfg.size(); ++i){
		std::string& type = cfg[i]["type"];
		len = mgm[type](midimsg, cfg[i]);
		dump(midimsg, len);
		mcs.add(midimsg, len);
	}  
	
#ifdef LINUX
	RaspiGPIO* gpio = new RaspiGPIO();
	gpio->setCallback(0, gpio_port0_triggered);
	gpio->setCallback(1, gpio_port1_triggered);
	start();
#else
	midiCtrlSet.run(midiOut);
#endif

#ifdef LINUX
	delete gpio;
#endif

	delete midiOut;
	return 0;          // so might be a good idea to erase it after closing.
}

///////////////////////////////////////////////////////////////////////////

