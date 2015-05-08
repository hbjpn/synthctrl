namespace generic{

inline int noteon(unsigned char* _buf, unsigned char channel, unsigned char note, unsigned char vel)
{
	_buf[0] = 0x90 + channel;
	_buf[1] = note;
	_buf[2] = vel;
	return 3;
}

inline int noteoff(unsigned char* _buf, unsigned char channel, unsigned char note, unsigned char vel)
{
	_buf[0] = 0x80 + channel;
	_buf[1] = note;
	_buf[2] = vel;
	return 3;
}

}
