# 42_webserv

## Before starting
- RFC 9110 - HTTP Semantics

### What is the HTTP
The Hypertext Transfer Protocol (HTTP) is a stateless application-level protocol for distributed, collaborative, hypertext information system. Simply to said that it is a protocol for fetching resources such as HTML document. It is the foundation of any data exchange on the Web and it is a client-server protocol, which means requests are initiated by the recipient, usually the Web browser. A documents is reconstructed from the different sub-documents fetched, for instance, text, layout desciption, images, videos, scripts, and more.

![image](https://github.com/bankung7/42_webserv/assets/65214132/6e311822-0eb5-4143-ab15-68eb5ac7084c)

HTTP was created for the World Wide Web (WWW) architecture and has evolved over time to support the scalability needs of a wordlwide hypertext system.

### Terminology

#### Resources
The target of an HTTP request is called a "resource". HTTP does not limit the nature of a resource. Most of it is identified by a Uniform Resource Identifier (URI). HTTP relies upon the URI standard to indicate the taret resource and relationship between resources.

#### Connections Clients and Servers
HTTP is a client/server protocol that operates over a reliable transport or session-layer "connection".
A HTTP client is a program that establishes a conenction to a server for the purpose of sending one or more HTTP requests.
A HTTP server is a program that accepts connections in order to service HTTP requests by sending HTTP responses.
HTTP is defined as a statless protocol, meaning that each request messages's semantics can be understood in isolation, and that the relationship between connections and messages on them has no impact on the interpretation of those messages.

#### User Agents
It refers to any of the various client programs that initiate a request.

#### Proxy
A Proxy is a message-forwarding agent that is chosen by the client, usually via local configuration rules, to receive requests for some type of absolute URI and attempt to satisfy those requests via translation through the HTTP interface.

#### Gateway
It is (a.k.a reverse proxy) an intermidiary that acts as an origin server for the outbound connection but translates received requests and forwards them inbound to another server.

#### Tunnel
A tunnel acts as a bind relay between two connections without changing the message. Once active, a tunnel is not considered a party to the HTTP communication, through the tunnel might have been initiaited by an HTTP request.

#### Cache
A cache is a local store of previous response messages and the subsystem that controls its message storage, retrival, and deletion.

#### Example of Message Exchange
Client Request:
```
GET / HTTP/1.1
User-Agent: curl/7.64.1
Host: www.example.com
Accept-Language: en, mi
```

Server Response:
```
HTTP/1.1 200 OK
Date: Mon, 27 Jul 2009 12:28:53 GMT
Server: Apache
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
ETag: "34aa387-d-1568eb00"
Accept-Ranges: bytes
Content-Length: 51
Vary: Accept-Encoding
Content-Type: text/plain

Hello World! My content includes a trailing CRLF.
```

### What is the Web server?
It term of Web server can refer to hardware or software, or both of them working together.
- Hardware side, a web server is a computer that stores web server software and a website's component files. A web server connects to the Internet and supports physical data interchange with other devices connected to the web.
- Software side, it includes several parts that control how web users access hosted files. An HTTP server is software that understands URLs (web address) and HTTP (the protocol your browser uses to view webpages). It can be accessed through the domain names of the websites it stores, and delivers the content of these hosted websites to the end user's device.

At most basic, whenever  a browser needs a file that is hosted on a web server, the browser request the file via HTTP. When the request reaches the correct web server, the HTTP server accepts the request, finds the requested document, and sends it back to the browser, through HTTP. It case the server can't find the requested document, it returns a 404 response instead.

![image](https://github.com/bankung7/42_webserv/assets/65214132/38a8f472-45b8-48dd-b204-3b94b0476757)

To publish a website, we need eithera static or a dynamic web server.

- A Static web server, or stack, consists of a computer with an HTTP server. We call it static because the server sends its hosted files as-is to your browser.
- A Dynamic web server consists of a static web server plus extra software, most commonly an application server and a database. We call it dynamic because the application server updates the hosted files before sending content to your browser via the HTTP server.

### Message Parsing
The normal procedure for parsing an HTTP message is to read the start-line into a structure, read each header field line into a hash table by field name until the empty line, and then use the parsed data to determine if a message body is expected. If a messagebody has been indicated, then it is read as a stream until an amount of octets equal to the message body length is read or the connection is closed.

### Request Line
```
request-line = method [SP] request-target [SP] HTTP-version
```
Remark: [SP] is single space

#### Method
The method token indicates the request method to be performed on the target resource. The request method is case-sensitive. It indicate the purpose for which the client has made this request and what is expected by the client as a successful result.

| Method Name | Description |
-
| GET | Transfer a current representation of the target resource |
| POST | Perform resource-specific processing on the request content |
| DELETE | Remove all current representation of the target resource |

There are more on the method like HEAD, PUT, CONNECT, OPTIONS, and TRACE but we will leave it as later.
The RFC 9110 are also said that the general-purpose server MUST support the methods GET and HEAD, All the other methods are OPTIONAL.

#### Request Target
The client derives a request-target from its desired target URI. There are 4 formats for the request-target, depending on both the method being requested and whether the request is to a proxy.

```
request-target = origin-form
                / absolute-form
                / authority-form
                / asterisk-form
```

No WHITESPACE is allowed in the request-target. Recipients of an invalid request-line SHOULD respond with either a 400 (Bad Request) or a 301 (Moved Permanently).
A client MUST send a Host header field in all HTTP/1.1 request-message. A server MUST respond with a 400 status code to any HTTP/1.1 request message that lacks a Host header field and to any request message that contains more than 1 Host header field line or a Host header field with an invalid field value.

##### origin-form
```
origin-form = absolute-path [ "?" query ]
```
A client MUST send only the absolute path and query components of the target URI as the request-target when making a request "**DIRECTLY**" to an origin server, other than a CONNECT or server-wide OPTIONS request. If the target URI's path component is empty, the client MUST send "/" as the path within the origin-form of request-target.
```
https://www.example.com/where?q=now
```
A client wishing to retrieve a representation of the resource directly from the origin server would open a TCP connection to port 80 of the host "www.example.com" and send the lines:
```
GET /where?q=now HTTP/1.1
Host: www.example.com
```

##### absolute-form
When making a request to a proxy, other than a CONNECT or server-wide OPTIONS request, a client MUST send the target URI in "absolute-form" as the request-target.
```
GET http://www.example.org/pub/WWW/TheProject.html HTTP/1.1
```

##### authority-form
It is only used for CONNECT requests. It consists of only the uri-host and port number of the tunnel desitnation, separated by a colon.
```
CONNECT www.example.com:80 HTTP/1.1
Host: www.example.com
```

##### asterisk-form
It is only used for a server-wide OPTIONS requests.
```
OPTIONS * HTTP/1.1
Host: www.example.org:8001
```

#### Status Line
the first line of a response message is the status-line, consisting of the protocol version, a space, the status code, and another space and ending with an OPTIONAL phrase describing the status code.
```
 status-line = HTTP-version SP status-code SP [ reason-phrase ]
```
The status code element is a 3-digit integer code describing the result of the server's attempt to understand and satisfy th client's corresponding request.

#### Field Syntax
Each field line consists of a case-insensitive field name followed by a colon, optional leading whitespace the field line value, and optional trailing whitespace
```
 field-line   = field-name ":" OWS field-value OWS
```

##### Field Line Parsing
Messages are parsed using a generic alforithm, independent of the individual field names. The contents within a given filed line value are not aprsed until a later stage of message interpretation (usually after the message's entire field section has been processed).
No WHITESPACE is allowed between the field name and colon, a server MUST reject with a status 400.
