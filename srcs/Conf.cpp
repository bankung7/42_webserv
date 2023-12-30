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
					_filedata.push_back('\n');
					_filedata = ltrim(rtrim(_filedata, " \t\r"), " \t\r");
					break;
				}
			}
		}
		ifs.close();
	}
	else
		std::cout << "Error: configuration file not found." << std::endl;
	std::cout << _filedata << std::endl;
	if (!(this->checkbraces()))
		std::cout << "Error: Invalid configuration file - Incorrect braces" << std::endl;
}

bool Conf::checkbraces(void) {
	int n_brace = 0;
	bool location_flag = false;
	std::string location = "";
	std::vector<std::string> lines = split(_filedata, '\n');
	for (size_t i = 0; i < lines.size(); i++)
	{
		if (!location_flag)
		{
			std::vector<std::string> words = split(lines[i], ' ');
			if (words[0] == "location" && words[2] == "{" && words.size() == 3) // location / {
			{
				location_flag = true;
				location = words[1];
			}	
			else if (words[0] == "server" && words[1] == "{" && words.size() == 2) // server {
				n_brace++;
			else if (words[0] == "}" && words.size() == 1) // } for server block
			{ 
				n_brace--;
				if (n_brace < 0)
					std::cout << "Error: wrong braces" << std::endl;
				else if (n_brace == 0)
					_n_server++;
			}
		}
		else
		{
			if (lines[i] == "}") // } for location block
			{	
				location = "";
				location_flag = false;	
			}
			else
			{

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
	std::cout << sv_conf << std::endl;
	// Server sv;
	// std::vector<std::string> lines = split(sv_conf, ';');
	// for (size_t i = 0; i < lines.size(); i++) {
	// 	std::vector<std::string> words = split(lines[i], ' ');
	// 	if (words[0] == "listen")
	// 	{
	// 		if (words.size() != 2 || isvalidport(words[1]) == -1)
	// 			return ;
	// 		else
	// 			sv.set_port(isvalidport(rtrim(words[1], ";")));
	// 		std::cout << "port: " << sv.get_port() << std::endl;
	// 	}
	// 	else if (words[0] == "server_name")
	// 	{
	// 		for (size_t i = 1; i < words.size(); i++)
	// 			sv.add_server_name(words[i]);
	// 		std::cout << "server_name: " << sv.get_server_name("webserv1") << std::endl;
	// 		std::cout << "server_name: " << sv.get_server_name("webserv2") << std::endl;
	// 	}
	// 	else if (words[0] == "client_max_body_size")
	// 	{
	// 		size_t max_size = ft_stoi(words[1]);
	// 		if (words.size() != 2)
	// 			return ;
	// 		else
	// 		{
	// 			sv.set_max_client_body_size(max_size);
	// 			std::cout << "max_size: " << sv.get_max_client_body_size() << std::endl;
	// 		}
	// 	}
	// 	else if (words[0] == "root")
	// 	{
	// 		if (words.size() != 2)
	// 			return ;
	// 		else
	// 			sv.set_root(words[1]);
	// 		std::cout << "root: " << sv.get_root() << std::endl;
	// 	}
    // }
}

// void Conf::checklocation(Server& sv) {
	
// }