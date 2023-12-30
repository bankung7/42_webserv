#include "Conf.hpp"

Conf::Conf(void) : _filename("./config/default.conf"), _filedata(""), _n_server(0){
	readfile();
}


Conf::Conf(std::string filename) : _filename(filename), _filedata(""), _n_server(0) {
	readfile();
	std::cout << _filedata << std::endl;
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
					// _filedata.push_back('\n');
					_filedata = ltrim(rtrim(_filedata, " \t\r\n"), " \t\r\n");
					break;
				}
			}
		}
		ifs.close();
	}
	else
		std::cout << "Error: configuration file not found." << std::endl;
	// std::cout << _filedata << std::endl;
	if (!(this->checkbraces()))
		std::cout << "Error: Invalid configuration file - Incorrect braces" << std::endl;
}

bool Conf::checkbraces(void) {
	int n_brace = 0;
	size_t sv_start = 8;
	if (_filedata.find("server {") != 0)
		return false;
	for (size_t i = 0; i < _filedata.size(); i++)
	{
		if (_filedata[i] == '{')
			n_brace++;
		else if (_filedata[i] == '}')
		{
			n_brace--;
			if (n_brace < 0)
			{
				_n_server = 0;
				return false;
			}
			else if (n_brace == 0)
			{
				_n_server++;
				size_t next_server = _filedata.find("server {", i, 8);
				if(next_server == i + 1)
				{
					checkserverconf(_filedata.substr(sv_start, i - sv_start));
					sv_start = i + 9;
				}
				else if (next_server == std::string::npos && i == _filedata.size() - 1)
					checkserverconf(_filedata.substr(sv_start, i - sv_start));
				else
					return false;
			}	
		}
	}
	if (n_brace != 0)
	{
		_n_server = 0;
		return false;
	}
	return true;
}

void Conf::checkserverconf(std::string sv_conf) {
	Server sv;
	std::vector<std::string> lines = split(sv_conf, ';');
	for (size_t i = 0; i < lines.size(); i++) {
        // std::cout << "--" << lines[i] << std::endl;
		std::vector<std::string> words = split(lines[i], ' ');
		if (words[0] == "listen")
		{
			if (words.size() != 2 || isvalidport(words[1]) == -1)
				return ;
			else
				sv.set_port(isvalidport(rtrim(words[1], ";")));
			std::cout << "port: " << sv.get_port() << std::endl;
		}
		else if (words[0] == "server_name")
		{
			for (size_t i = 1; i < words.size(); i++)
				sv.add_server_name(words[i]);
			std::cout << "server_name: " << sv.get_server_name("webserv1") << std::endl;
			std::cout << "server_name: " << sv.get_server_name("webserv2") << std::endl;
		}
		else if (words[0] == "client_max_body_size")
		{
			size_t max_size = ft_stoi(words[1]);
			if (words.size() != 2 || max_size < 0)
				return ;
			else
			{
				sv.set_max_client_body_size(max_size);
				std::cout << "max_size: " << sv.get_max_client_body_size() << std::endl;
			}
		}
		else if (words[0] == "root")
		{
			if (words.size() != 2)
				return ;
			else
				sv.set_root(words[1]);
			std::cout << "root: " << sv.get_root() << std::endl;
		}
		else if (words[0] == "location")
		{

		}
    }
}

void Conf::checklocation() {
	
}