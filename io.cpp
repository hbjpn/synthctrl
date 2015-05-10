#include "picojson.h"
#include "io.h"
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
	return true; 
}

void tomap(const picojson::value::object& obj, std::map<std::string, std::string>& m)
{
	picojson::value::object::const_iterator c_it = obj.begin();
	for(; c_it != obj.end(); ++c_it){
		const std::string& key = c_it->first;
		const picojson::value& v = c_it->second;
		if(!v.is<std::string>()){
			continue;
		}
		m[key] = v.get<std::string>();
	}
}

void plainfy(picojson::value& v, config& a)
{
	picojson::value::array& arr = v.get<picojson::array>();
	picojson::value::array::const_iterator c_it = arr.begin();
	for(; c_it != arr.end(); ++c_it){
		std::map<std::string, std::string> m;
		tomap(c_it->get<picojson::object>(), m);
		a.push_back(m);
	}
}

void loadConfig(const char* fn, config& cfg)
{
	picojson::value v;
	loadJSON(fn, v);
	plainfy(v, cfg);
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
