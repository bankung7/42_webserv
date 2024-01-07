#include "Conf.hpp"

Conf::Conf(void) :
	_filename("./config/default.conf"),
	_filedata(""),
	_n_server(0),
	_am_flag(false),
	// _root_flag(false),
	// _groot_flag(false),
	_port_flag(false) {
	readfile();
}


Conf::Conf(std::string filename) :
	_filename(filename),
	_filedata(""),
	_n_server(0),
	_am_flag(false),
	// _root_flag(false),
	// _groot_flag(false),
	_port_flag(false) {
	readfile();
}

Conf::~Conf(void) {

}

// read file, trim, ignoring empty line and comment
void Conf::readfile(void) {
    std::ifstream	ifs(this->_filename.c_str());
	std::string     line;
	if (ifs.is_open())
	{
		while (std::getline(ifs, line))
		{
			if (line.find('#') < line.length())
				line.erase(line.find('#'));
			for (std::string::iterator it = line.begin(); it < line.end(); it++)
			{
				if (!isspace(*it))
				{
					_filedata.append(it, line.end());
					_filedata = trim(_filedata, " \n\t\r");
					_filedata.push_back('\n');
					break;
				}
			}
		}
		ifs.close();
	}
	else
		throw std::runtime_error("Error: Configuration file not found.");
	if (!(this->checkconf()))
		throw std::runtime_error("Error: Invalid configuration file");
	// std::cout << "OK!! The configuration file is valid." << std::endl;
}

bool Conf::checkconf(void) {
	int n_brace = 0;
	bool location_flag = false;
	bool server_flag = false;
	std::string svcf = "";
	std::vector<std::string> lines = split(_filedata, "\n");
	for (size_t i = 0; i < lines.size(); i++)
	{
		if (!server_flag)
		{
			std::vector<std::string> words = split(lines[i], " \t\n\r");
			if (checkserver(words))
			{
				server_flag = true;
				n_brace++;
			}
			else if (checkemptyline(words)) {;}
			else
			{
				_n_server = 0;
				return false;
			}
		}
		else
		{
			if (!location_flag) // outside the location block
			{
				std::vector<std::string> words = split(lines[i], " \t\n\r");
				if (checklocation(words)) // location / {
				{
					location_flag = true;
					n_brace++;
					svcf.append(rtrim(lines[i], "{"));
				}
				else if (checkmaxclientsize(words) || checkservername(words) || checkerrorpage(words) \
					|| checkport(words)) {
					if (lines[i][lines[i].length() - 1] == ';')
						lines[i] = trim(lines[i], ";");
					svcf.append(lines[i]+'\n');
				}
				else if (checkclosebraces(words)) { // } for server block
					n_brace--;
					server_flag = false;
					if (n_brace < 0)
					{
						_n_server = 0;
						return false;
					}
					if (!this->_port_flag)
						return false;
					this->_port_flag = false;
					_n_server++;
					_serverconf.push_back(rtrim(svcf, "\n"));
					svcf = "";
				}
				else if (checkemptyline(words)) {;}
				else
				{
					_n_server = 0;
					return false;
				}
			}
			else // inside the location block
			{
				std::vector<std::string> words = split(lines[i], ":");
				if (checkclosebraces(words)) // } for location block
				{
					n_brace--;
					if (n_brace < 0)
					{
						_n_server = 0;
						return false;
					}
					if (!this->_am_flag)
						return false;
					this->_am_flag = false;
					location_flag = false;
					svcf.append(rtrim(lines[i], "}") + '\n');
				}
				else if (checkallowedmethods(words) || checkroot(words) || checkreturn(words) \
					|| checkallowfileupload(words) || checkautoindex(words) || checkindex(words) \
					|| checkerrorpageinlocation(words) || checkmaxclientsize(words) || checkuploadpath(words)) {
					if (lines[i][lines[i].length() - 1] != ';')
						lines[i].append(";");
					svcf.append(lines[i]);
				}
				else
				{
					_n_server = 0;
					return false;
				}
			}
		}
	}
	if (n_brace != 0 || _n_server != _serverconf.size())
	{
		_n_server = 0;
		return false;
	}
	return true;
}

void Conf::parseconf(std::vector<Server> &svs) {
	for (size_t i = 0; i < _n_server; i++)
	{
		Server sv;
		std::vector<std::string> lines = split(_serverconf[i], "\n");
		for (size_t j = 0; j < lines.size(); j++) {
			std::vector<std::string> words = split(lines[j], " \t\n\r");

            //// debug
            // if (words[0].compare("location") == 0)
            //     std::cout << "check word size: " << words.size() << " => " << words[1] << std::endl;

            // for (int j = 0; j < (int)words.size(); j++) {
            //     std::cout << j << " : " << words[j] << std::endl;
            // }

            // //// debug

			// set port
			if (checkport(words)) {sv.set_port(isvalidport(words[1]));}
			// set server name
			else if (checkservername(words)) {
				for (size_t k = 1; k < words.size() - 1; k++)
					sv.add_server_name(words[k]);
			}
			// set root
			else if (checkroot(words)) {sv.set_root(words[1]);}
			// set location
			else if (words[0] == "location") {
				for (size_t k = 3; k < words.size(); k++)
					words[2].append(" ").append(words[k]);
				sv.set_location(words[1], words[2]);
			}
			// set max client body size
			else if (checkmaxclientsize(words)) {sv.set_max_client_body_size(ft_stost(words[1]));}
		}
		svs.push_back(sv);
	}
}

//////////// Check Directives ////////////
bool Conf::checkroot(std::vector<std::string> words) {
	return (words[0] == "root" && words.size() == 2);
}

bool Conf::checkmaxclientsize(std::vector<std::string> words) {
	return (words[0] == "client_max_body_size" && words.size() == 2 && ft_stoi(words[1]) > 0);
}

bool Conf::checkport(std::vector<std::string> words) {
	if (words[0] == "listen" && words.size() == 2 && isvalidport(words[1]))
	{
		this->_port_flag = true;
		return true;
	}
	return false;
}

bool Conf::checkservername(std::vector<std::string> words) {
	return (words[0] == "server_name" && words.size() > 1);
}

bool Conf::checkemptyline(std::vector<std::string> words){
	return (words[0] == "" && words.size() == 1);
}

bool Conf::checkallowedmethods(std::vector<std::string> words){
	if (words[0] == "allowedMethod" && words.size() > 1)
	{
		words[1] = trim(words[1], ";");
		std::vector<std::string> methods = split(words[1], ",");
		for (size_t i = 0; i < methods.size(); i++)
		{
			if (methods[i] != "GET" && methods[i] != "POST" && methods[i] != "DELETE")
				return false;
		}
		this->_am_flag = true;
		return true;
	}
	else
		return false;
}

bool Conf::checkerrorpage(std::vector<std::string> words){
	if (words[0] == "error_page" && words.size() > 2) {
		for (size_t i = 1; i < words.size() - 1; i++)
		{
			if (!isvalidstatuscode(words[i]))
			{
				_n_server = 0;
				return false;
			}
		}
	}
	else
		return false;
	return true;
}

bool Conf::checkerrorpageinlocation(std::vector<std::string> words){
	if (words[0] == "error_page" && words.size() == 2) {
		std::vector<std::string> statuscode = split(words[1], " ");
		for (size_t i = 0; i < statuscode.size() - 1; i++)
		{
			if (!isvalidstatuscode(statuscode[i]))
			{
				_n_server = 0;
				return false;
			}
		}
	}
	else
		return false;
	return true;
}

bool Conf::checkclosebraces(std::vector<std::string> words){
	return (words[0] == "}" && words.size() == 1);
}

bool Conf::checkreturn(std::vector<std::string> words) {
	return ((words[0] == "return"));
}

bool Conf::checkallowfileupload(std::vector<std::string> words) {
	if (words[0] == "allowedFileUpload" && words.size() == 2) {
		if (words[1] == "yes;" || words[1] == "no;")
			return true;
		else
			return false;
	}
	else
		return false;
}

bool Conf::checkautoindex(std::vector<std::string> words) {
	if (words[0] == "autoIndex" && words.size() == 2) {
		if (trim(words[1], " ;\t\n\r") == "on" || trim(words[1], " ;\t\n\r") == "off")
			return true;
		else
			return false;
	}
	else
		return false;
}

bool Conf::checkserver(std::vector<std::string> words) {
	return (words[0] == "server" && words[1] == "{" && words.size() == 2);
}

bool Conf::checklocation(std::vector<std::string> words) {
	return (words[0] == "location" && words[2] == "{" && words.size() == 3);
}

bool Conf::checkindex(std::vector<std::string> words) {
	return(words[0] == "index" && words.size() == 2);
}

bool Conf::checkuploadpath(std::vector<std::string> words) {
	return(words[0] == "uploadPath" && words.size() == 2);
}


//////////// Getter////////////
size_t Conf::get_n_server() {
	return (this->_n_server);
}

std::string Conf::get_serverconf(size_t n) {
	if (n > _serverconf.size() - 1)
		return (NULL);
	return (this->_serverconf[n]);
}