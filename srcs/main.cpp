#include "webserv.h"

int main(void) {
    
    Server wb;

}

// Step
// Create socket
// bind socket with port
// Listen incoming message
// Accept request
// Parse the message
// // check method [GET POST DELETE]
// // check file type [html jpeg js ...]
// // open the file and read
// // send file
// close socket


// HTTP request Example
// GET / HTTP/1.1
// Host: localhost:8080
// Connection: keep-alive
// sec-ch-ua: "Chromium";v="116", "Not)A;Brand";v="24", "Google Chrome";v="116"
// sec-ch-ua-mobile: ?0
// User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36
// sec-ch-ua-platform: "Windows"
// Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8
// Sec-Fetch-Site: same-origin
// Sec-Fetch-Mode: no-cors
// Sec-Fetch-Dest: image
// Referer: http://localhost:8080/
// Accept-Encoding: gzip, deflate, br
// Accept-Language: en-US,en;q=0.9

// HTTP response Example
// HTTP/1.1 200 OK              => status line
// Content-type: image/jpeg     => Header
// Server: IBM....              => Header
// Date: Thu, 02 Sep 2023 ..    => Header
// ...                          => Body
