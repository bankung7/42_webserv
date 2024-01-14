# Webserv [EPOLL]

## Requirements
- Accept configuration file as argument, use default if not defined.
- Must be non-blocking and use only 1 poll() for all the I/O operations (listen include).
- poll() must check read and write at the same time.
- Never do read or write operations without going through poll().
- Checking the errno() is forbidden after read or write.
- Don't need to use poll() before reading configuration file.
- Request should never hang forever.
- Web Broswer Compatible to any of our choice
- Consider NGINX is HTTP 1.1, compare header and answer to this behavior.
- HTTP response status code must be accurate.
- Must have deault error pages, if none are provided.
- Can't use fork for anything else than CGI.
- Serve a fully statis website.
- Client can upload file, and download it back.
- Compatible with GET, POST and DELETE method
- Stress test at lest 99.5%, never die at all cost.
- Must be able to listen to multiple port (defined in config file). 

## PROGRESS
[C] Parsing Configuration file \
[C] Server setup and Initializing \
[C] Epoll Server \
[C] Handling request \
[C] Handling response \
[C] CGI \
[C] Error Handling\
[C] Static Web \

### Specifications
[C] root directive \
[C] allowedMethod directive \
[C] autoIndex directive \
[C] index directive \
[C] return directive \
[C] Location Directive \
[C] maxClientBodyLimit directive \
[C] error_page directive \
[C] GET request \
[C] POST request \
[C] DELETE request \
[C] Uploading file [multipart-formdata] \
[C] Downloading file \
[C] Connection keep-alive \
[C] Response Code checking \

#### update: 14/01/24 09:05

## Flowchart
https://app.diagrams.net/#G1HveiOX7h5U4jHvzp7YklAA40VSXkAxFU

## Configuration file
```
server {

    listen 8080                             // mandatory
    server_name webserv1 webserv2           // need to set in /etc/hosts
    client_max_body_size 1000               // 
    root /var/www                           // mandatory, for fallback case
    index index.html                        // defautl page when directory is request

    location / {
        allowedMethod:GET,POST,DELETE;      // list of accepted HTTP methods
        return 301 https://www.hotmail.com; // HTTP redirection
        root:/sites/www1;                   // rot directory
        autoindex:on;                       // directory listing
        allowedUploadFile:yes;              // allowed for upload file
        uploadPath:/sites/www1/temp;        // default path of uploaded file
        error_page: 404 /404.html;          // set default error page
        error_page: 500 501 502 /50x.html;  // set default error page
    }

    location /cgi-bin/ {
        allowedMethod:GET,POST;
    }

    location /temp/ {
        allowedMethod:GET,POST;
        root:/sites/www1;
        autoIndex:on;
    }
}
```

## Binding the port
```
SO_REUSEADDR
It allows to reuse the local address and port for the socket

SO_REUSEPORT
It allows multiple socket to bind to the same local address and port.
```

### Location Directive

```
location location_match {
    ...
}
```
The location_match defines what it should check the request URI against. This will not take any modifier like nginx.
This is interpreted as a prefix match. It means that the location given will be matched against the beginning of the request URI to determine the match.
#### Example
```
location / {
    root:/var/www;
    error_page 404 /site/404.html;
}

location /site {
    root:/var/www/other;
}
```

if the request is made for /foo, the first location block will be called. it will try to find a file called "foo" in the "/var/www" directory.
However, if a file is not found (a 404 status), an internal redirect to "/error/404.html" will occur, leading to a new location search that will land on the second block and try to find "/var/www/other/site/404.html".
if the request is made for /site/index.html, the second location will be called. It will try to find a file called "index.html" in the /var/www/other/site/index.html" directory.

This project will apply to non-modifier only!!!!.

## URI anatomy
```
https://example.org:8080/foo/bar?q=baz#bang

protocol - https
hostname - example.org                    // name of device that connect to the network
port     - 8080
host     - example.org:8080
origin   - https://example.org:8080
pathname - /foo/bar
search   - ?q=bax
href     - https://example.org:8080/foo/bar?q=baz
```

> [!NOTE]
> The /etc/hosts file is a plain text file used in matching a fully qualified domain name (FQDN) with the server IP hosting a specific domain. When the DNS server cannot be reached, the /etc/hosts file to resolve the domain name.

### Directory Listing
This use autoindex directive to show. The webserv will try to display a directory listing when a particular URL path ends with a /. Example, the url is /assets/css/, it will look in the folder /css/ to see if it can find an index.html file. if the autoindex is not active and there is no index.html exist, it will return 404.

## HTTP Request

## HTTP Response
### Header
```
HTTP/1.1 200 OK\r\n
Content-type: text/html\r\n
Content-Length: 1230\r\n\r\n
```

### GET Method
This method does not send the payload in body part but it put all the payload as query parameter to its URL, even we use form tag.

```
<form action="/goto/" method="GET" enctype="...">
    <input type="hidden" name="username" value="test">
    <input type="hidden" name="password" value="test">
    <input class="btn-submit" type="submit" value="form-data">
</form>

http://webserv1:8080/goto/?username=test&password=test
```

### POST Method
This section will use the form tag in html.

#### Form-Data
each "input" tag in html will be splited by boundary. The boundary is in the request header => Content-Type
```
<form action="/goto/" method="POST" enctype="multipart/form-data">
    <input type="hidden" name="username" value="test">
    <input type="hidden" name="password" value="test">
    <input class="btn-submit" type="submit" value="form-data">
</form>

------WebKitFormBoundary95ZYculdXgPumcMP
Content-Disposition: form-data; name="uploadFile"; filename="12.txt"
Content-Type: text/plain

TEST for /test1/index.html

------WebKitFormBoundary95ZYculdXgPumcMP
Content-Disposition: form-data; name="option1"

value1
------WebKitFormBoundary95ZYculdXgPumcMP
Content-Disposition: form-data; name="option2"

value2
------WebKitFormBoundary95ZYculdXgPumcMP--
```

#### urlencoded
```
<form action="/goto/" method="POST" enctype="application/x-www-form-urlencoded">
    <input type="hidden" name="username" value="test">
    <input type="hidden" name="password" value="test">
    <input class="btn-submit" type="submit" value="urlencoded">
</form>

username=test&password=test
```

#### text/plain
```
<form action="/goto/" method="POST" enctype="text/plain">
    <input type="hidden" name="username" value="test">
    <input type="hidden" name="password" value="test">
    <input class="btn-submit" type="submit" value="text/plain">
</form>

username=test
password=test

```

### Redirection

for 300 series
```
return (301 | 302 | 303 | 307) url;
```

for other series
```
return (1XX | 2XX | 4XX | 5XX) ["text"];
```

## CGI
This will only accept for GET and POST method.

### GET
It received a parameter by query string which passing through QUERY_STRING in tehe environment (execve)
What we have to do is to extract the query section from the URL and put it in the environment variable.

### POST
There a 3 types of POST, which the book said that it will send the data to CGI via STDIN. That mean we have to pipe it. (WTG)

https://www.tutorialspoint.com/python/python_cgi_programming.htm
https://www.ibm.com/docs/en/netcoolomnibus/8.1?topic=scripts-environment-variables-in-cgi-script

## Concept
### Parsing Configuration file
### Initialize Servers
### Polling
### Read the Request
### Send the Response

## Tips
- to set the /etc/hosts in ~window~ run this powershell command 
    Start-Process -FilePath notepad.exe -Verb runas -ArgumentList "$env:SystemRoot\system32\drivers\etc\hosts"
