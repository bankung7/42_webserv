# 42_webserv

# Basic Concepts
## HTTP Protocal
The Hypertext Transfer Protocol (HTTP) is an application-level protocol for distributed, collaborative, hypermedia information systems. This is the foundation for data communication for the World Wide Web (WWW). HTTP is a generic and stateless protocol which can be used for other purposes as wll using extensions of its request methods, error codes, and headers.
Basically, HTTP is a TCP/IP based communication protocol, that is used to deliver data (HTML files, images file, query results, etc.) on the WWW. The default port is TCP 80, but other ports can be used as well. It provides a standardized way for computers to communicate with each other. HTTP specification specifies how client's request data will be constructed and sent to the server, and how the servers respond to these requests.

### HTTP is connectionless
The HTTP client, eg. a browser, initiates an HTTP request and after a request is made, the client waits for the response. The server processes the request and sends a response back after which client disconnect the connection. So client and server knows about each other during current request and response only. Further requests are made on new connection like client and server are new to each other.

### HTTP is media independent
It means, any type of data can be sent by HTTP as long as both the client and the server know how to handle the data content. It is required for the client as well as server to specify the content type using appropriate MIME-type.

### HTTP is stateless
HTTP is connectionless and it is direct result of HTTP being a stateless protocol. The server and client are aware of each other only during a current request. Afterwards, both of them forget about each other. Both the client and the browser can not retain information between different requests across the web pages.

## Client-Server Architecture
### Basic Architecture
![image](https://github.com/bankung7/42_webserv/assets/65214132/116df624-fa6e-4e65-a8ff-0f0aa6edf2e3)

The HTTp protocol is a  request/response protocol based on the client/server based architecture where web browsers, robots and serarch engines act like HTTP clients and the Web server acts as a server.

### Client
The HTTP client sends a request to the server in the form of a request method, URI, and protocol version, followed by a MIME-like message containing request modifiers, client information, and possible body content over a TCP/IP connection.

### Server
The HTTP server responds with a status line, including the message's protocol version and a success or error code, followed by MIME-like message containing server information, entity meta information, and possible entity-body content.

## Server-side and Client-side
### Server-side
Server-side refers to processes that are carried out on the web server, where the website or web application is hosted. These processes are typically executed by the server before the website or web application is delivered to the user's device, and they can include tasks such as retrieving data from a database, rendering a web page, or handling user input.

### Client-side
Client-side, on the other hand, refers to processes that are carried out on the user's device, typically in the user's web browser. These processes are executed after the website or web application has been delivered to the user's device, and they can include task such as rendering a web page, handling user interactions, or running JavaScript code.

## Web Server?
A web server is software and hardware that uses HTTP and other protocol to respond to client requests made over the WWW. The main job of a web server is to display website content through storing, processing and delivering webpages to users.

Web server hardware is connected to the internet and allows data to be exchanged with other connected devices, while web server software controls how a user accesses hosted files. The web server process is an example of the client/server model. All computers that host websites must have web server software.

### How do web servers work?
Web server software is accessed through the domain names of websites and ensures the delivey of the site's content to the requesting user. The software side is also comprised of several components, with at least an HTTP server. The HTTP server is able to understand HTTP and URLs. As hardware, a web server is a computer that stores web server software and other files related to a website, such as HTML documents, images and JavaScript files.
When a web browser needs a file that is hosted on a web server, the browser will request the file by HTTP. When the request is received by the web server, the HTTP server will accept the request, find the content and send it back to browser through HTTP.
- Browser obtain the IP address of the domain name
- Transalte the URL through DNS (Domain Name System) or by searching in its caches.
- Browser request the specific file from the web server by HTTP request.
- Web server respond, sending the requested page through HTTP.
- Browser displays the webpage

Web servers are used in web hosting, or the hosting of data for websites and web-based applications -- or web application.

Web servers also support SMTP (Simple Mail Transfer Protocol) and FTP (File Transfer Protocol), used for email, file transfer and storage.

### Static web server
It refers to the content being shown as is. A static web server will consist of a computer and HTTP software. It is considered static because the server will send hosted file as is to a browser.

#### Static Content
It is any file that is stored in a server and is the same everytime it is delivered to users. HTML files and images are examples of this kind of content. It is like a newspaper: once it was published, it features the same articles and photos all day for everyone who pick up a copy.

### Dynamic web server
It consist of a web server and other software such as an application server and database. It is considered dynamic because the application server can be used to update any hosted files before they are sent to a browser. The web server can generate content when it is requested from database. Though this process is more flexible.

### Common and top web server software on the market
- Apache HTTP server.
- Microsoft Internet Information Services (IIS).
- NGINX.
- Lighttpd.
- Sun Java System Web Server.

## HTTP Requests and Responses
### HTTP Request
An HTTP client sends an HTTP request to a server in the form of a request message which includes following format:
```
Request Line
Header
[Empty line]
Message-body [OPTIONAL]
```

#### Request Line
It begin with a method token, followed by the Request-URI and the protocol version, and ending with CRLF. The elements are separated by space [SP] characters.
```
Request-Line = Method SP Request-URI SP HTTP-Version CRLF
```

#### Request Method
The request method indicates the method to be performed on the resource identified by the given Request-URI. The method is CASE-SENSITIVE and should always be mentioned in UPPERCASE.

| Method | Description |
|--------|-------------|
| `GET` | It is used to retrieve information from the given server using a given URI. Request using GET should only retrieve data and should have no other effect on the data. |
| `HEAD` | Same as GET, but it transfers the status line and the header section only. |
| `POST` | It is used to send data to the server, for example, customer information, file upload, etc. using HTML forms. |
| `PUT` | Replaces all the current representations of the target resource with the uploaded content. |
| `DELETE` | Removes all the current representation of the target resource given by URI. |
| `CONNECT` | Establishes a tunnel to the server identified by given URI. |
| `OPTIONS` | Describe the communication options for the target resource. |
| `TRACE` | Performs a message loop back test along with the path to the target resource. |

We will only focus on GET, POST, and DELETE in this project.

#### Request-URI
The Request-URI identifies the resource upon which to apply the request.
```
Request-URI = "*" | absoluteURI | abs_path | authority
```

| Method | Description |
| - | - |
| * | It is used when an HTTP request does not apply to a particular resource, but to the server itself, and is only allowed when the method used does not neccessarily apply to a resource. |
| absoluteURI | It is used when an HTTP request is being made to a **proxy**. The proxy is requested to  forward the request or service from a valid cache, and return the response. |
| absolute_path | It is the most common form of Request-URI is that used to identify a resource on an origin server or gateway. It can not be empty; if none is present in the original URI, it MUST be given as "/" (the server root). |

#### Request Header Fields
This field allow the client to pass additional information about the request, and about the client itseld, to the server. These fields act as request modifiers. Here is a list of some important header fields.

- Accept : it can be used to specify certain media types which are acceptable for the response.
  ```
  Accept: type/subtype [q=value]
  Accept: text/plain; text/html, text/x-dvi;
  ```

- Accept-Charset : it can be userd to indicate what character sets are acceptable for the response.
  ```
  Accept-Charset: character_set [q=value]
  Accept-Charset: unicode1-1;
  ```
- Accept-Encoding : It is similar to Accept, but restrict the content-coding that are acceptable in the response.
  ```
  Accept-Encoding: encoding types
  Accept-Encoding: compress, gzip
  ```
- Accept-Language : It is similar to Accept, but restrict the set of nature languages that are preferred as a response to the request.
  ```
  Accept-Language: language [q=value]
  Accept-Language: da, en-gb;
  ```
- Authorization : It consists of credentials containing the authentication information of the user agent for the realm of the resource being request.
  ```
  Authorization: creadentials
  Authorization: BASIC Z3Vlc3Q6Z3Vlc3QxMjM=
  ```
- Cookie : It contains a name/value pair of information stored for that URL.
  ```
  Cookie: name=value;name=value; ...
  ```
- Expect
- From
- Host
- If-Match
- If-Modified-Since
- If-None-Match
- If-Range
- If-Unmodified-Since
- Max-Forwards
- Proxy-Authorization
- Range
- Referer
- TE
- User-Agent

#### Example of Request Message
```
GET /hello.htm HTTP/1.1
User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)
Host: www.tutorialspoint.com
Accept-Language: en-us
Accept-Encoding: gzip, deflate
Connection: Keep-Alive
```

```
POST /cgi-bin/process.cgi HTTP/1.1
User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)
Host: www.tutorialspoint.com
Content-Type: application/x-www-form-urlencoded
Content-Length: length
Accept-Language: en-us
Accept-Encoding: gzip, deflate
Connection: Keep-Alive

licenseID=string&content=string&/paramsXML=string
```

```
POST /cgi-bin/process.cgi HTTP/1.1
User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)
Host: www.tutorialspoint.com
Content-Type: text/xml; charset=utf-8
Content-Length: length
Accept-Language: en-us
Accept-Encoding: gzip, deflate
Connection: Keep-Alive

<?xml version="1.0" encoding="utf-8"?>
<string xmlns="http://clearforest.com/">string</string>
```

### HTTP Response
After receiving and interpreting a request message, a server responds with an HTTP response message.
```
Status line
Header
[Empty line]
Message-body [OPTIONAL]
```

#### Message Status Line
It consists of the protocol version followed by a numeric status code and its associated textual phrase. The elements are separated by space [SP] characters.
```
Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
```
#### HTTP Version
the supporting version of HTTP, 1.1, it will return 
```
HTTP-Version = HTTP/1.1
```

#### Status Code
It is a 3-digit integer where first digit of the code defines the class of response and the last 2 digits do not have any categorization role. There are 5 values for the first digit:

| Code | Description |
| - | - |
| `1XX` | Information: it means the request was received and the process is continuing. |
| 100 | Continue: Only a part of the request has been received by the server, but as long as it has not been rejected, the client should continue with request. |
| `2XX` | Success: it means the action was successfully received, understood, and accepted. |
| 201 | Created : The request is completed, and a new resource is create. |
| 202 | Accepted : The request is accepted for processing, but the processing is not completed. |
| `3XX` | Redirection: it means further action must be taken in order to complete the request. |
| 301 | Moved Permanently : The requested page has moved to a new URL. |
| `4XX` | Client Error: it means the request contains incorrect syntax or cannot be fulfilled. |
| 400 | Bad Request : The server did not understand the request. |
| 401 | Unthorized : The requested page needs a username and a password. |
| 403 | Forbidden : Access is forbidden to the requested page. |
| 404 | Not Found : The server can not find the requested page. |
| 408 | Request Timeout | The request took longer than the server was prepared to wait. |
| `5XX` | Server Error: it means the server failed to fulfill an apparently valid request. |
| 500 | internal Server Error : The request was not completed. The server met an unexpected condition. |
| 502 | Bad Gateway : The request was not completed. The server received an invalid response from the upstream server. |
| 503 | Service Unavailable : The request was not completed. The server is temporarily overloading or down. |
| 504 | Gateway Timeout : The dateway has time out. |

Those are some the most code we see, for more detail : https://developer.mozilla.org/en-US/docs/Web/HTTP/Status

#### Response Header Fields
It allows the server to pass additional information about the response which cannot be placed in the Status-Line. These header fields give information about the server and about further access to the resource identifier by the Request-URI.
- Accept-Ranges
- Age
- ETag
- Location
- Proxy-Authenticate
- Retyr-After
- Server
- Vary
- WWW-Authenticate

#### Example of Response Message
```
HTTP/1.1 200 OK
Date: Mon, 27 Jul 2009 12:28:53 GMT
Server: Apache/2.2.14 (Win32)
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
Content-Length: 88
Content-Type: text/html
Connection: Closed
```

```
HTTP/1.1 404 Not Found
Date: Sun, 18 Oct 2012 10:36:20 GMT
Server: Apache/2.2.14 (Win32)
Content-Length: 230
Connection: Closed
Content-Type: text/html; charset=iso-8859-1
```

```
HTTP/1.1 400 Bad Request
Date: Sun, 18 Oct 2012 10:36:20 GMT
Server: Apache/2.2.14 (Win32)
Content-Length: 230
Content-Type: text/html; charset=iso-8859-1
Connection: Closed
```

For other, refer to https://www.tutorialspoint.com/http/http_header_fields.htm

## What is the socket?
A socket is a programming interface that provides a way for processes to communicate over a network using standard protocols. It's an abstraction that allows programs to send and receive data over a network connection. regardless of whether the communicatioin is happening within the same computer (inter-process communication) or between different computer (network communication).
`
So, Think of a socket as a virtual communication endpoint:
`

### Socket Types
There are 2 primary types of sockets:

#### Stream sockets (often associated with TCP)
These provide a reliable, ordered, and connection-oriented communication channel. Data sent using stram sockets is **guaranteed** to arrive in the same order it was sent, and if any data is lost or corrupted, the underlying protocol handles retransmission.

#### Datagram sockets (often associated with UDP)
These provide a connectionless and unreliable communication channel. Data sent using datagram sockets might arrive out of order, duplicated, ot not at all. However, UDP can be faster and is often used for real-time application like video streaming and online gaming.

### What Sockets are used for?
- Networking Application
- Inter-Process Communication (IPC)
- Client-Server Communication
- Data Exchange
- Network Protocols

### Summary
A socket is a mechanism that enables communication between processes or application over a network. It abstracts away many of the complexities of network communication and provides a common interface that programmers can use to build a wide range of network application.

## How Socket works?
Sockets are commonly used for client and server interaction. Typical system configuration places the server on one machine, with the clients on other machines. The client s connect to the server, exchange information, and then disconnect.
A socket has a typical flow of events. In a connection-oriented client-to-server model, the socket on the server process waits for requests from a client. To do this, the server first establishes (binds) an address that clients can use to fine the server. When the address is established, the server waits for clients to request a service. The client-to-server data exhanges takes place when a client connects to the server throught a socket. The server performs the client's requests and sends the reply back to the client.

![image](https://github.com/bankung7/42_webserv/assets/65214132/1c29b755-52ba-4b90-be8a-ffc18fcf3a6d)

### Socket Characteristics
- A socket is represented by an integer called a **socket descriptor**
- A socket exists as long as the process maintains an open link to the socket.
- We can name a socket and use it to communicate with other sockets in a communication domain.
- Sockets perform the communication when the server accepts connections from them, or when it exchanges messages with them.
- We can create sockets in pairs (only for AF_UNIX address family).

### Socket Address Family
- AF_INET : provides interprocess communication between processes that run on the same system or on different systems.
- AF_INET6 : provides support for the Internet Protocol version 6 (IPv6), uses a 128 bit address.
- AF_UNIX : provides interprocess communication on the same system that uses the socket APIs.
- AF_UNIX_CCSID : is compatible with the AF_UNIX and has the same limitations.

### Socket Type

#### Stream (SOCK_STREAM)
This type is connection-oriented. Establish an end-to-end connection by using the bind(), listen(), accept(), and connect() APIs. SOCK_STREAM sends data without error or duplication, and receives the data in the sending order.

#### Datagram (SOCK_DGRAM)
THs basic unit of data transfer is a datagram. The datagram socket is connectionless, establish no end-to-end connection with the transport provider(protocol). The socket sends datagrams as independent packets with no guarantee of delivery. We might lose or duplicate data. It also might arrive out of order. The size is limited ti the data size that we can send in a single transaction.

#### Raw (SOCK_RAW)
This type of socket allows direct access to lower-layer protocols. It requires more programming expertise because we manage the protocol header information used by the transport provider. At this level, the transport provider can dictate the format of the data and the semantic that are transport-provider specific.

## HTTP Request
The steps involved in the HTTP requests are:
1. Client send the request
2. Handshaking is done between the client and server, connection is established
3. Server process response
4. Server send response back to client
5. Close the connection

Every HTTP request is considered a new request because every request creates a new connection, as soon as the response is returned the connection is closed. In most web application rest API use HTTP requests.

## HTTP Polling
This is a variation of HTTP request where a client keeps sending a request to the server at a regular interval and the response can be empty.
The step has the same as an HTTP request but only some difference is multiple requests at regular intervals.
1. Client send the request.
2. Handshaking is done between the client and server, connection is established
3. Server process response.
4. Server send the response to the client, it may or may not have any update for the client so the response can be empty.
5. Close the connection.
6. This process is repeated at regular intervals to fetch the response of change from the server.

The major disadvantange of this is unnecessary network calls as most of the time the response will be empty.

## EPOLL
It is a linux kernel system call for as scalable I/O event notification mechanism. It is similar to 'kqueue' which consists of a set of user-space function, each taking a file descriptor. Epoll uses a red-black tree data structure to keep track of all fd that are currently being monitor.

```
int epoll_create(int size); # size is ignored but must be more that 0;
```
This funciton returns its fds.

```
int epoll_ctl(int epfd, int op, in fd, struct epoll_event *event);
```
this function contorls which fds are watched by this object, and for which event.
'op' can beว
EPOLL_CTL_ADD : add fd to the interest list
EPOLL_CTL_MOD : change the setting with fd in the interest list
EPOLL_CTL_DEL : remove the target fd from the interest list.

```
int epoll_wait(int epfd, struct epoll_event *events, int max_events, int timeout);
```
It waits for any of the event registered for with epoll_ctl, until at least one occurs or the time out elapses. Returns the occurede events in 'events' up to max_events at once. 'max_events' is the maximum number of 'epoll_event' fd to be monitored. In most case, max_events is set to the value of the sizeof '*event' arguments.
EPOLLIN : file is available for read operations;
EPOLLOUT : file is available for write operations;
EPOLLERR : error condition happened on the fd;
EPOLLET : set the edge triggered;

### Triggering modes
edge-triggered will return only when a new event is enqueued with the epoll object.
level-triggered will return as long as the condition holds.
