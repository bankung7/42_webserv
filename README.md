# Webserv [EPOLL]

## PROGRESS
### Configuration file
- [ ] Structure
- [ ] Parsing

### Request Handler
- [ ] Structure
- [ ] Parsing the request

### Response Handler
- [ ] Create the response
- [ ] Response Status
- [ ] When connection is not "keep-alive"

### Server Specification
- [x] Epoll process
- [x] Choose port and host
- [x] Setup server_name
- [x] default server for host:port
- [ ] default error page
- [ ] Limit Client Body Size
- [ ] Location rules
- [ ] Allowed Method
- [ ] Redirection
- [ ] Root
- [ ] Directory Listing
- [ ] Default file if Directory
- [ ] Upload and Download file

### CGI
- [ ] cgi

### Testing
- [ ] Stress

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
We must manually input the host name in /etc/host. This will be matching the ip with host for server_name.
We use 127.0.0.1 for all testing.

### Listen
It will accept the port int numeric format only.
Each server block must have only 1 listen.
> listen 8080;

To use same port, put ~SO_REUSEPORT~ while binding.

### Server Name
If server_name is specify by host in the request.
> server_name wwww.example.com

```
The first server for a host:port will be the default for this host:port (that means
it will answer to all the requests that don’t belong to an other server).
```

#### Result ####
Different port, same server_name:\
    - match port, then find exact server_name.

Different port, different server_name:\
    - match port, then find exact server_name.

Different port, no server_name:\
    - match port, return default server block.

Same port, different server_name:\
    - when found the exact server_name, return that block.\
    - when not found the server_name, return default server block.

Same port, same server_name:
    - return the first found (default server block).

## Something found
- when siege with 0.0.0.0, it stuck with on idea what it is.
- to set the /etc/hosts in ~window~ run this powershell command 
    Start-Process -FilePath notepad.exe -Verb runas -ArgumentList "$env:SystemRoot\system32\drivers\etc\hosts"
