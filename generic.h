#include "util.h"

namespace generic{

inline int noteon(unsigned char* _buf, unsigned char channel, unsigned char note, unsigned char velocity)
{
	_buf[0] = 0x90 + channel;
	_buf[1] = note;
	_buf[2] = velocity;
	return 3;
}

inline int noteon(unsigned char* _buf, std::map<std::string, std::string>& argv)
{
	return noteon(_buf, AsByte(argv["channel"]), AsByte(argv["note"]), AsByte(argv["velocity"]));
}



inline int noteoff(unsigned char* _buf, unsigned char channel, unsigned char note, unsigned char velocity)
{
	_buf[0] = 0x80 + channel;
	_buf[1] = note;
	_buf[2] = velocity;
	return 3;
}

inline int noteoff(unsigned char* _buf, std::map<std::string, std::string>& argv)
{
	return noteoff(_buf, AsByte(argv["channel"]), AsByte(argv["note"]), AsByte(argv["velocity"]));
}


}
