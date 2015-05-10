#include <map>
#include <string>

#include "util.h"

namespace korg{

inline int zone(unsigned char* _buf, unsigned char track, char lu, unsigned char value)
{
	unsigned char buf[18] = {
		0xF0, 66, 48, 0, 1, 21, 65, 0, 0, 12, 0, 52, 0, 0, 0, 0, 0, 0xf7};
	buf[9 ] = 12 + track;
	buf[11] = (lu == 'l') ? 52 : 49; 
	buf[16] = value;
	memcpy(_buf, buf, 18);
	return 18;
}

inline int zone(unsigned char* _buf, std::map<std::string, std::string>& argv)
{
	return zone(_buf, AsByte(argv["track"]), AsChar(argv["lu"]), AsByte(argv["position"]));
}

inline int solo(unsigned char* _buf,
	unsigned char track,
	unsigned char value)
{
	unsigned char buf[18] = {
		0xF0, 66, 48, 0, 1, 21, 65, 0, 0, 12, 0, 41, 0, 0, 0, 0, 0, 0xf7};
	buf[9 ] = 12 + track;
	buf[16] = value;
	memcpy(_buf, buf, 18);
	return 18;
}

inline int solo(unsigned char* _buf, std::map<std::string, std::string>& argv)
{
	return solo(_buf, AsByte(argv["track"]), AsByte(argv["value"]));
}

inline int mute(unsigned char* _buf,
	unsigned char track,
	unsigned char value)
{
	unsigned char buf[18] = {
		0xF0, 66, 48, 0, 1, 21, 65, 0, 0, 12, 0, 40, 0, 0, 0, 0, 0, 0xf7};
	buf[9 ] = 12 + track;
	buf[16] = value;
	memcpy(_buf, buf, 18);
	return 18;
}

inline int mute(unsigned char* _buf, std::map<std::string, std::string>& argv)
{
	return mute(_buf, AsByte(argv["track"]), AsByte(argv["value"]));
}

inline int tone(unsigned char* _buf,
	unsigned char bank_corse,
	unsigned char bank_fine,
	unsigned char program){
	unsigned char buf[8] = {
		0xb0, 0, 0, 0xb0, 32, 0, 0xc0, 0};
	buf[2] = bank_corse;
	buf[5] = bank_fine;
	buf[7] = program;
	memcpy(_buf, buf, 8);
	return 8;
};

inline int tone(unsigned char* _buf, std::map<std::string, std::string>& argv)
{
	return tone(_buf, AsByte(argv["bank_corse"]), AsByte(argv["bank_fine"]), AsByte(argv["program"]));
}

}
