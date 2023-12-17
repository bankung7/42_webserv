# Webserv [EPOLL]

## PROGRESS
[N] Configuration file structure & parsing.
[N] Request Handler
    [N] Structure
    [N] Parsing the request
[N] Response Handler
    [N] Create the response
        [N] Response Status
    [N] When connection is not "keep-alive"
[I] Server Specification
    [C] Epoll process
    [I] Choose port and host
    [I] Setup server_name
    [I] default server for host:port
    [N] default error page
    [N] Limit Client Body Size
    [N] Location rules
        [N] Allowed Method
        [N] Redirection
        [I] Root
        [N] Directory Listing
        [N] Default file if Directory
    [N] Upload and Download file
[N] CGI
[N] Stress Test

### Terminology
[N] - Not starting.
[I] - In progress
[C] - Completed, wait for testing.
[T] - Testing.
[P] - Testing Success and got the expected result.
[F] - Testing Success and fail.

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
[Guess] I think if all the server block routed to the same port but diferrent in server_name for each block. It will check the exact match to each block and use that block to process. If not it will use the first server block for default.

## Something found
- when siege with 0.0.0.0, it stuck with on idea what it is.
- to set the /etc/hosts in ~window~ run this powershell command 
    Start-Process -FilePath notepad.exe -Verb runas -ArgumentList "$env:SystemRoot\system32\drivers\etc\hosts"