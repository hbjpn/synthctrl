typedef std::vector< std::map<std::string, std::string> > config;

bool loadConfig(const char* fn, config& cfg);

bool loadJSON(const char* fn, picojson::value& v);
