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
Get /hello.txt HTTP/1.1
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
