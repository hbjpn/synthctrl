#include <alsa/asoundlib.h>     /* Interface to the ALSA system */
#include <unistd.h>             /* for sleep() function */

#include "korg.h"

// function declarations:
void errormessage(const char *format, ...);

///////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   int status;
   int mode = SND_RAWMIDI_SYNC;
   snd_rawmidi_t* midiout = NULL;
   const char* portname = "hw:1,0,0";  // see alsarawportlist.c example program
   if ((argc > 1) && (strncmp("hw:", argv[1], 3) == 0)) {
      portname = argv[1];
   }
   if ((status = snd_rawmidi_open(NULL, &midiout, portname, mode)) < 0) {
      errormessage("Problem opening MIDI output: %s", snd_strerror(status));
      exit(1);
   }

   unsigned char sysex[18]; // =  {0xF0, 66, 48, 0, 1, 21, 65, 0, 0, 12, 0, 52, 0, 0, 0, 0, 29, 0xf7};
   korg::zone(sysex, 0, 'l', 64);
   char noteon[3]  = {0x90, 60, 100};
   char noteoff[3] = {0x90, 60, 0};


   if ((status = snd_rawmidi_write(midiout, sysex, 18)) < 0) {
      errormessage("Problem writing to MIDI output: %s", snd_strerror(status));
      exit(1);
   }

/*
   sleep(1);

   if ((status = snd_rawmidi_write(midiout, noteon, 3)) < 0) {
      errormessage("Problem writing to MIDI output: %s", snd_strerror(status));
      exit(1);
   }

   sleep(1);  // pause the program for one second to allow note to sound.

   if ((status = snd_rawmidi_write(midiout, noteoff, 3)) < 0) {
      errormessage("Problem writing to MIDI output: %s", snd_strerror(status));
      exit(1);
   }
*/
   snd_rawmidi_close(midiout);
   midiout = NULL;    // snd_rawmidi_close() does not clear invalid pointer,
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


