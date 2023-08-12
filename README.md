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
-
| **GET** | It is used to retrieve information from the given server using a given URI. Request using GET should only retrieve data and should have no other effect on the data. |
| HEAD | Same as GET, but it transfers the status line and the header section only. |
| **POST** | It is used to send data to the server, for example, customer information, file upload, etc. using HTML forms. |
| PUT | Replaces all the current representations of the target resource with the uploaded content. |
| **DELETE** | Removes all the current representation of the target resource given by URI. |
| CONNECT | Establishes a tunnel to the server identified by given URI. |
| OPTIONS | Describe the communication options for the target resource. |
| TRACE | Performs a message loop back test along with the path to the target resource. |

We will only focus on GET, POST, and DELETE in this project.

#### Request-URI
The Request-URI identifies the resource upon which to apply the request.
```
Request-URI = "*" | absoluteURI | abs_path | authority
```

| Method | Description |
-
| * | It is used when an HTTP request does not apply to a particular resource, but to the server itself, and is only allowed when the method used does not neccessarily apply to a resource. |
| absoluteURI | It is used when an HTTP request is being made to a **proxy**. The proxy is requested to  forward the request or service from a valid cache, and return the response. |
| absolute_path | It is the most common form of Request-URI is that used to identify a resource on an origin server or gateway. It can not be empty; if none is present in the original URI, it MUST be given as "/" (the server root). |

#### Request Header Fields
This field allow the client to pass additional information about the request, and about the client itseld, to the server. These fields act as request modifiers. Here is a list of some important header fields.

- Accept-Charset
- Accept-Encoding
- Accept-Language
- Authorization
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
_
| 1XX | Information: it means the request was received and the process is continuing. |
| | 100 | Continue: Only a part of the request has been received by the server, but as long as it has not been rejected, the client should continue with request. |
| 2XX | Success: it means the action was successfully received, understood, and accepted. |
| 3XX | Redirection: it means further action must be taken in order to complete the request. |
| 4XX | Client Error: it means the request contains incorrect syntax or cannot be fulfilled. |
| 5XX | Server Error: it means the server failed to fulfill an apparently valid request. |

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


