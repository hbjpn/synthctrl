#include <alsa/asoundlib.h>     /* Interface to the ALSA system */
#include <unistd.h>             /* for sleep() function */

#include "generic.h"
#include "korg.h"

// function declarations:
void errormessage(const char *format, ...);

///////////////////////////////////////////////////////////////////////////
class MIDIOut{
	snd_rawmidi_t* _midiout;
public:
	MIDIOut(const char* portname){
		int status;
		int mode = SND_RAWMIDI_SYNC;
		_midiout = NULL;
   		if ((status = snd_rawmidi_open(NULL, &_midiout, portname, mode)) < 0) {
			errormessage("Problem opening MIDI output: %s", snd_strerror(status));
      			exit(1);
   		}
	}

	void send(const unsigned char* buf, int len){
		int status;
		if ((status = snd_rawmidi_write(_midiout, buf, len)) < 0) {
			errormessage("Problem writing to MIDI output: %s", snd_strerror(status));
			exit(1);
		}
	}

	virtual ~MIDIOut()
	{
		snd_rawmidi_close(_midiout);
		_midiout = NULL;    // snd_rawmidi_close() does not clear invalid pointer,
	} 
};


int main(int argc, char *argv[]) {
	const char* portname = "hw:1,0,0";  // see alsarawportlist.c example program
	if ((argc > 1) && (strncmp("hw:", argv[1], 3) == 0)) {
		portname = argv[1];
	}

	MIDIOut* midiOut = new MIDIOut(portname);

	unsigned char midimsg[128];
	int len;
		
   	len = korg::zone(midimsg, 0, 't', 127);
	midiOut->send(midimsg, len);   
  			
   	len = korg::zone(midimsg, 1, 'l', 126);
	midiOut->send(midimsg, len);   
  	
   	len = korg::zone(midimsg, 2, 'l', 126);
	midiOut->send(midimsg, len);   
   	
   	len = korg::zone(midimsg, 3, 'l', 126);
	midiOut->send(midimsg, len);   
  
/*
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

//////////////////////////////
//
// error -- Print an error message.
//

void errormessage(const char *format, ...) {
   va_list ap;
   va_start(ap, format);
   vfprintf(stderr, format, ap);
   va_end(ap);
   putc('\n', stderr);
}


