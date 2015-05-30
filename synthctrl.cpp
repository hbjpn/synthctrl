//#include <alsa/asoundlib.h>     /* Interface to the ALSA system */

#include <unistd.h>             /* for sleep() function */
#include <pthread.h>
#include <syslog.h>
#include <queue>
#include <signal.h>

#include "RtMidi.h"
#include "picojson.h"

#include "generic.h"
#include "korg.h"

#include "io.h"
#ifdef LINUX
#include "raspi_gpio.h"
#include "socket.h"
#endif

// typedefs
typedef int (*midigen)(unsigned char*, std::map<std::string, std::string>&);

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

class CtrlSet
{
    enum CtrlSeqType{
        MIDI = 0,
        WAIT_GPIO0,
        WAIT_GPIO1,
        TIMER
    };
    struct CtrlSeq{
        CtrlSeqType type;
        union {
            std::vector<unsigned char>* midiMsg;
            long long int duration_ns;
        };
    };
    std::vector<CtrlSeq> _cs;
    int _idx;
    RtMidiOut* _midiOut;
    SocketInterface* _si;
public:
    CtrlSet(){};
    void setMidiOut(RtMidiOut* midiOut){
        _midiOut = midiOut;
    }
    void init(const char* fn, std::map<std::string, midigen>& mgm, SocketInterface* si){
        clear();
        _si = si;
	    config cfg;
	    bool ret = loadConfig(fn, cfg);
        if(!ret){
            syslog(LOG_NOTICE,"Failed to load config file %s.", fn);
            return;
        }
	    for(int i = 0; i < cfg.size(); ++i){
	    	std::string& type = cfg[i]["type"];
            CtrlSeq seq;
            if(type == "WAIT_GPIO0"){
                seq.type = WAIT_GPIO0;
            }else if(type == "WAIT_GPIO1"){
                seq.type = WAIT_GPIO1;
            }else if(type == "TIMER"){
                seq.type = TIMER;
                seq.duration_ns = AsInt64(cfg[i]["duration"]);
            }else{
                seq.type = MIDI;
                seq.midiMsg = new std::vector<unsigned char>();
                unsigned char rawmsg[128];
                int len = mgm[type](rawmsg, cfg[i]);
                seq.midiMsg->assign(&rawmsg[0], &rawmsg[len]);
            }
            _cs.push_back(seq);
	    }  

    };
    virtual ~CtrlSet()
    {
        clear();
    }
    void clear(){
        for(int i = 0; i < _cs.size(); ++i){
            if(_cs[i].type == MIDI)
                delete _cs[i].midiMsg;
        }
        _cs.clear();
        _idx = 0;
    }
    void increment()
    {
        char buf[128];
        sprintf(buf,"p %d",_idx);
        _si->send(buf);
        ++_idx;
    }
    void proceed(){
        for(; _idx < _cs.size(); ){
            if(_cs[_idx].type == MIDI){
                _midiOut->sendMessage(_cs[_idx].midiMsg);
                dump(&(*_cs[_idx].midiMsg)[0], _cs[_idx].midiMsg->size());
            }else if(_cs[_idx].type == TIMER){
                setTimer(_cs[_idx].duration_ns, NULL, SIGRTMIN);
                break;
            }else{
                break;
            }
            increment();
        }
    };
    void notifyGPIO0(){
        if(_idx < _cs.size() && _cs[_idx].type == WAIT_GPIO0){
            increment();
            proceed();
        }
    };
    void notifyGPIO1(){
        if(_idx < _cs.size() && _cs[_idx].type == WAIT_GPIO1){
            increment();
            proceed();
        }
    }
    void notifyTimerExpire(){
        if(_idx < _cs.size() && _cs[_idx].type == TIMER){
            increment();
            proceed();
        }
    }
};


EventQueue* evt_queue = NULL;
CtrlSet cs;
std::map<std::string, midigen> mgm;


void* event_loop(void* arg)
{
	evt_queue = new EventQueue();
	SocketInterface* si = new SocketInterface("127.0.0.1", 6565, *evt_queue);
 
	bool loop_continue = true;
	while(loop_continue){
		const Event* evt = evt_queue->pop();

		/* process event */
		syslog(LOG_NOTICE, "Event triggered : %d", evt->type());

		switch(evt->type()){
		case EVT_GPIO0_TRIGERRED:
            cs.notifyGPIO0();
            break;
		case EVT_GPIO1_TRIGERRED:
		    cs.notifyGPIO1();
            break;
		case EVT_END_LOOP:
			loop_continue = false;
			break;
		case EVT_DEPLOY:{
			const EventDeploy* devt = dynamic_cast<const EventDeploy*>(evt);
			syslog(LOG_NOTICE, "Deploy : %s\n", devt->_fn.c_str());
			cs.init(devt->_fn.c_str(), mgm, si);
            break;
        }
        case EVT_TIMER_EXPIRED:
            cs.notifyTimerExpire();
            break;
		default:
			break;
		}
		delete evt;
	}
	fprintf(stderr, "End event loop thread\n");
	si->shutdown();
	si->join();
	delete evt_queue;
	evt_queue = NULL;
	return NULL;
}

void gpio_port0_triggered()
{
	Event* evt = new Event(EVT_GPIO0_TRIGERRED);
	evt_queue->push(evt);
}

void gpio_port1_triggered()
{
	Event* evt = new Event(EVT_GPIO1_TRIGERRED);
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
  for(int signo = SIGRTMIN ; signo <= SIGRTMAX; ++signo){
    ret = sigaddset(&ss, signo);  
  }
  ret = sigprocmask(SIG_BLOCK, &ss, NULL);
  if (ret != 0) 
    return false;

  while(1) {
    if (sigwait(&ss, &signo) == 0) {
      if(signo >= SIGRTMIN && signo <= SIGRTMAX){
        Event* evt = new Event(EVT_TIMER_EXPIRED);
        evt_queue->push(evt);
      }else{
         /* handle SIGHUP */
        Event* evt = new Event(EVT_END_LOOP);
        evt_queue->push(evt);
        break;
      }
    }
  }

  return true;
}


void start()
{
	pthread_t threads[1];
	pthread_create(&threads[0], NULL, event_loop, NULL);

	wait_signal(); // Ctrl+C is monitored
	syslog(LOG_NOTICE, "Waiting for end threads ... ");
	pthread_join(threads[0],NULL);
}

int mainLoop()
{       
    openlog("synthctrl", LOG_CONS|LOG_PID, LOG_USER); 
    syslog(LOG_NOTICE, "sysctrl start");

    // Create PID file
    const char* pidfile = "/var/run/synthctrl.pid";
    FILE* fp = fopen(pidfile,"w");
    if(!fp){
        syslog(LOG_ERR, "Failed to create pid file at %s", pidfile);
        closelog();
        return 0;
    }
    fprintf(fp,"%ld\n",(long int)getpid());
    fclose(fp);
     
    closelog();
	mgm["noteon"]     = generic::noteon;
	mgm["noteoff"]    = generic::noteoff;
	mgm["korg::zone"] = korg::zone;
	mgm["korg::tone"] = korg::tone;
	mgm["korg::solo"] = korg::solo;
	mgm["korg::mute"] = korg::mute;

	midiOut = new RtMidiOut();
	unsigned int nports = midiOut->getPortCount();
	int port = nports - 1; // Use last MIDI
	midiOut->openPort(port);

    cs.setMidiOut(midiOut);

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
    syslog(LOG_NOTICE, "syncthctrl exits normally");
    remove(pidfile);
    closelog();
	return 0;          // so might be a good idea to erase it after closing.

}

int main(int argc, char *argv[])
{
    if(daemon(0,0) == 0){
        mainLoop();
    } else{
        printf("Failed to make daemon\n");
    }
    return 0;
 
}

///////////////////////////////////////////////////////////////////////////

