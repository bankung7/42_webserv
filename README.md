# Webserv [EPOLL]

## PROGRESS
- [ ] Configuration file structure & parsing.
- [ ] Request Handler
&nbsp;&nbsp;- [ ] Structure
&nbsp;&nbsp;- [ ] Parsing the request
- [ ] Response Handler
&nbsp;&nbsp;- [ ] Create the response
&nbsp;&nbsp;- [ ] Response Status
&nbsp;&nbsp;- [ ] When connection is not "keep-alive"
- [ ] Server Specification
&nbsp;&nbsp;- [x] Epoll process
&nbsp;&nbsp;- [x] Choose port and host
&nbsp;&nbsp;- [x] Setup server_name
&nbsp;&nbsp;- [x] default server for host:port
&nbsp;&nbsp;- [ ] default error page
&nbsp;&nbsp;- [ ] Limit Client Body Size
&nbsp;&nbsp;- [ ] Location rules
&nbsp;&nbsp;- [ ] Allowed Method
&nbsp;&nbsp;- [ ] Redirection
&nbsp;&nbsp;- [ ] Root
&nbsp;&nbsp;- [ ] Directory Listing
&nbsp;&nbsp;- [ ] Default file if Directory
&nbsp;&nbsp;- [ ] Upload and Download file
- [ ] CGI
- [ ] Stress Test

## Configuration file

```
server {

    listen [port];
    server_name [name];
    error_page [path];
    client_max_body_size [int];

    location [path] {
        allowedMethod [GET,POST,DELETE];    // list of accepted HTTP methods
        return [url];                       // HTTP redirection
        root [directory];                   // directory
        autoindex [boolean];                // directory listing
        ???                                 // default file for directory request.
        ???                                 // cgi
        allowedUploadFile [boolean];
        uploadPath [path];
    }
}
```

### Host
We must manually input the host name in /etc/host. This will be matching the ip with host for server_name.\
We use 127.0.0.1 for all testing.\

### Listen
It will accept the port int numeric format only.\
Each server block must have only 1 listen.\
> listen 8080;

To use same port, put ~SO_REUSEPORT~ while binding.\

### Server Name
If server_name is specify by host in the request.\
> server_name wwww.example.com\

```
The first server for a host:port will be the default for this host:port (that means
it will answer to all the requests that don’t belong to an other server).
```
[Guess] I think if all the server block routed to the same port but diferrent in server_name for each block. It will check the exact match to each block and use that block to process. If not it will use the first server block for default.\

## Something found
- when siege with 0.0.0.0, it stuck with on idea what it is.\
- to set the /etc/hosts in ~window~ run this powershell command 
    Start-Process -FilePath notepad.exe -Verb runas -ArgumentList "$env:SystemRoot\system32\drivers\etc\hosts"\
