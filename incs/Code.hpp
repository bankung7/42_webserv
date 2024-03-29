#ifndef CODE_HPP
#define CODE_HPP

// Webservevr
#define MAX_EVENTS 1024
#define BACKLOG 100

// For Text Color
#define BLACK "\033[0;30m"
#define B_BLACK "\033[1;30m"
#define RED "\033[0;31m"
#define B_RED "\033[1;31m"
#define GREEN "\033[0;32m"
#define B_GREEN "\033[1;32m"
#define YELLOW "\033[0;33m"
#define B_YELLOW "\033[1;33m"
#define BLUE "\033[0;34m"
#define B_BLUE "\033[1;34m"
#define PURPLE "\033[0;35m"
#define B_PURPLE "\033[1;35m"
#define CYAN "\033[0;36m"
#define B_CYAN "\033[1;36m"
#define WHITE "\033[0;37m"
#define B_WHITE "\033[1;37m"
#define C_RESET "\033[0m"

// state
#define S_GREEN "\033[1;32m[DEBUG]: "
#define S_DEBUG "\033[0m[DEBUG]: "
#define S_INFO "\033[1;34m[INFO]: "
#define S_WARNING "\033[1;33m[WARNING]: "
#define S_ERROR "\033[1;31m[ERROR]: "
#define S_END "\033[0m\n"

// Variable
#define BUFFER_SIZE 1024

// Progess
#define NOTSTART 0
#define READING 1
#define READING_HEADER 2
#define READING_BODY 3
#define PROCESSING 4
#define CGI_OUT 5
#define CGI_IN 6
#define TRY_FILE_PHASE 7
#define CONTENT_PHASE 8
#define SENDING 9
#define COMPLETED_PHASE 10
#define CLOSED 99
#define ERROR -1


// POST Form type
#define URLENCODED 11
#define FORMDATA 12

// Keep alive time
#define KEEP_ALIVE_TIME_OUT 5

#endif