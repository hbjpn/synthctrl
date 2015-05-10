//#include <alsa/asoundlib.h>     /* Interface to the ALSA system */

#include <unistd.h>             /* for sleep() function */

#include "RtMidi.h"
#include "picojson.h"

#include "generic.h"
#include "korg.h"

#include "io.h"

///////////////////////////////////////////////////////////////////////////
void dump(unsigned char* buf, int len)
{
	for(int i = 0; i < len; ++i){
		fprintf(stderr, "0x%02x ", buf[i]);
	}
	fprintf(stderr, "\n");
}

typedef int (*midigen)(unsigned char*, std::map<std::string, std::string>&);

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

	for(int i = 0; i < cfg.size(); ++i){
		std::string& type = cfg[i]["type"];
		len = mgm[type](midimsg, cfg[i]);
		dump(midimsg, len);
		std::vector<unsigned char> msg(&midimsg[0], &midimsg[len]);
		midiOut->sendMessage(&msg);
	}  

/*		
   	len = korg::zone(midimsg, 0, 't', 127);
	midiOut->send(midimsg, len);   
  			
   	len = korg::zone(midimsg, 1, 'l', 126);
	midiOut->send(midimsg, len);   
  	
   	len = korg::zone(midimsg, 2, 'l', 126);
	midiOut->send(midimsg, len);   
   	
   	len = korg::zone(midimsg, 3, 'l', 126);
	midiOut->send(midimsg, len);   
  
	len = korg::solo(midimsg, 0, 1);
	midiOut->send(midimsg, len);   
 
	len = generic::noteon(midimsg, 0, 60, 100);
	midiOut->send(midimsg, len);	
	len = generic::noteoff(midimsg, 0, 60, 0);
	midiOut->send(midimsg, len);
	*/
	delete midiOut;
	return 0;          // so might be a good idea to erase it after closing.
}

///////////////////////////////////////////////////////////////////////////

