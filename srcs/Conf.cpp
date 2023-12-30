#include "Conf.hpp"

Conf::Conf(void) : _filename("./config/default.conf"), _filedata(""), _n_server(0){
	readfile();
}


Conf::Conf(std::string filename) : _filename(filename), _filedata(""), _n_server(0) {
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
		std::cout << "Error: configuration file not found." << std::endl;
	if (!(this->checkconf()))
		std::cout << "Error: Invalid configuration file" << std::endl;
	else
		std::cout << "OK" << std::endl;
}

bool Conf::checkconf(void) {
	int n_brace = 0;
	bool location_flag = false;
	bool server_flag = false;
	std::vector<std::string> lines = split(_filedata, '\n');
	for (size_t i = 0; i < lines.size(); i++)
	{
		if (!server_flag)
		{
			std::vector<std::string> words = split(lines[i], ' ');
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
			if (!location_flag)
			{
				std::vector<std::string> words = split(lines[i], ' ');
				if (checklocation(words)) // location / {
				{
					location_flag = true;
					n_brace++;
				}	
				else if (checkmaxclientsize(words)){;}
				else if (checkservername(words)) {;}
				else if (checkerrorpage(words)) {;}
				else if (checkport(words)) {;}
				else if (checkroot(words)) {;}
				else if (checkclosebraces(words)) {
					n_brace--;
					server_flag = false;
					if (n_brace < 0)
					{
						_n_server = 0;
						return false;
					}
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
				std::vector<std::string> words = split(lines[i], ':');
				if (checkclosebraces(words)) // } for location block
				{
					n_brace--;
					if (n_brace < 0)
					{
						_n_server = 0;
						return false;
					}
					location_flag = false;	
				}
				else if (checkallowedmethods(words)) {;}
				else if (checkroot(words)) {;}
				else if (checkreturn(words)) {;}
				else if (checkallowfileupload(words)) {;}
				else if (checkautoindex(words)) {;}
				else if (checkindex(words)) {;}
				else
				{
					_n_server = 0;
					return false;
				}
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

bool Conf::checkroot(std::vector<std::string> words) {
	return (words[0] == "root" && words.size() == 2);
}

bool Conf::checkmaxclientsize(std::vector<std::string> words) {
	return (words[0] == "client_max_body_size" && words.size() == 2 && ft_stoi(words[1]) > 0);
}

bool Conf::checkport(std::vector<std::string> words) {
	return (words[0] == "listen" && words.size() == 2 && isvalidport(words[1]));
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
		std::vector<std::string> methods = split(words[1], ',');
		for (size_t i = 0; i < methods.size(); i++)
		{
			if (methods[i] == "GET" || methods[i] == "POST" || methods[i] == "DELETE")
				return true;
			else
				return false;
		}
		return false;
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

bool Conf::checkclosebraces(std::vector<std::string> words){
	return (words[0] == "}" && words.size() == 1);
}

bool Conf::checkreturn(std::vector<std::string> words) {
	return ((words[0] == "return" && words.size() == 2));
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
	if (words[0] == "autoindex" && words.size() == 2) {
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


// void Conf::checkserverconf(std::string sv_conf) {
// 	std::cout << sv_conf << std::endl;
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
// }
