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

// Variable
#define BUFFER_SIZE 1024

// Progess
#define NOTSTART 0
#define READING 1
#define WRITING 2
#define COMPLETED 3
#define CLOSED 99

#define POST_READ_PHASE 11
#define FIND_SERVER_PHASE 12
#define FIND_LOCATION_PHASE 13
#define CREATE_RESPONSE_PHASE 14
#define TRY_FILE_PHASE 15
#define CONTENT_PHASE 16
#define COMPLETE_PHASE 17
#define NO_SERVER_FOUND -1

// POST Form type
#define URLENCODED 11
#define FORMDATA 12

// Keep alive time
#define KEEP_ALIVE_TIME_OUT 5

#endif