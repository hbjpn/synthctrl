#include "picojson.h"
#include <iostream>
#include <fstream>

bool loadJSON(const char* fn, picojson::value& v)
{
	std::ifstream ifs(fn);
	if(!ifs)
		return false;
	int len;
	ifs.seekg(0, std::ios::end);
	len = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	char* buf = new char[len+1];
	ifs.read(buf, len);
	buf[len] = '\0';
	std::string s(buf);
	
	std::string err = picojson::parse(v, s);
	if( !err.empty() ){
		std::cerr << err << std::endl;
		return false;
	}
	delete [] buf; 
}
/*
int main(int argc, char** argv)
{
	picojson::value v;
	loadJSON(argv[1], v);

	if(!v.is<picojson::array>()){
		return 0;
	}
	
	const picojson::value::array& arr = v.get<picojson::array>();
	for(picojson::value::array::const_iterator it = arr.begin();
		it != arr.end();
		++it){
		
	}
		
	return 0;
}
*/
