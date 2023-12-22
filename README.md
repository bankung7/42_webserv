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
= Unknown =

## Flowchart
https://app.diagrams.net/#G1HveiOX7h5U4jHvzp7YklAA40VSXkAxFU

## Configuration file
```
server {
    listen 8080;                            // mandatory
    server_name webserv1;                   // need to set in /etc/hosts
    error_page 404 /404.html;               // default error page
    client_max_body_size 1000;              // 
    root /var/www/;                         // mandatory, for fallback case
    index index.html                        // defautl page when directory is request

    location / {
        allowedMethod GET POST DELETE;      // list of accepted HTTP methods
        return webserv2:8082;               // HTTP redirection
        root /var/www/;                     // rot directory
        autoindex on;                       // directory listing
        allowedUploadFile on;               // allowed for upload file
        uploadPath /var/www/uploads;        // default path of uploaded file
    }

    location cgi-bin {
        root ./;                             // mandatory, cgi-bin location
        cgi_path /usr/bin/python3 /bin/bash; // mandatory, interpreter location
        cgi_extension                        // mandatory, accepted cgi extension
    }
}
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
    root /var/www;
    error_page 404 /site/404.html;
}

location /site {
    root /var/www/other;
}
```

if the request is made for /foo, the first location block will be called. it will try to find a file called "foo" in the "/var/www" directory.
However, if a file is not found (a 404 status), an internal redirect to "/error/404.html" will occur, leading to a new location search that will land on the second block and try to find "/var/www/other/site/404.html".
if the request is made for /site/index.html, the second location will be called. It will try to find a file called "index.html" in the /var/www/other/site/index.html" directory.

## Location anatomy
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

## Concept
### Parsing Configuration file
### Initialize Servers
### Polling
### Read the Request
### Send the Response

## Tips
- to set the /etc/hosts in ~window~ run this powershell command 
    Start-Process -FilePath notepad.exe -Verb runas -ArgumentList "$env:SystemRoot\system32\drivers\etc\hosts"
