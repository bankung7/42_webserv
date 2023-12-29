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
					_filedata = ltrim(rtrim(_filedata));
					_filedata.push_back('\n');
					break;
				}
			}
		}
		ifs.close();
	}
	else
		std::cout << "Error: configuration file not found." << std::endl;
	if (!(this->checkbraces()))
		std::cout << "Error: Invalid configuration file - Incorrect braces"<< std::endl;
	checkdirectives();
}

bool Conf::checkbraces(void) {
	int n_brace = 0;
	std::string::iterator it;
	if (_filedata.find("server") != 0)
		return false;
	for (std::string::iterator it = _filedata.begin(); it < _filedata.end(); it++)
	{
		if (*it == '{')
			n_brace++;
		else if (*it == '}')
		{
			n_brace--;
			if (n_brace < 0)
			{
				_n_server = 0;
				return false;
			}
			else if (n_brace == 0)
				_n_server++;
		}
	}
	if (n_brace != 0)
	{
		_n_server = 0;
		return false;
	}
	return true;
}

bool Conf::checkdirectives() {
	std::vector<std::string> lines = split(_filedata, '\n');
	for (size_t i = 0; i < lines.size(); i++) {
        std::vector<std::string> words = split(lines[i], ' ');
		if (words[0] == "server")
		{
			if (words.size() != 2 || words[1] != "{")
				return false ;
		}
		if (words[0] == "listen")
		{
			if (words.size() != 2 || isvalidport(words[1]) == -1)
				return false;
		}
		if (words[0] == "server_name")
		{

		}
		if (words.size() != 2 || )
		if (words[0] == "client_max_body_size")

        _result.push_back(words);
    }
	return true;
}