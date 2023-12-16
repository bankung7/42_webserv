# Webserv [EPOLL]

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

### Listen
It will accept the port int numeric format only.
Each server block must have only 1 listen.
This will be bind to 0.0.0.0
> listen 8080;

### Server Name
If server_name is specify by host in the request.
This server_name must be manually added in the /etc/hosts.
> server_name wwww.example.com

## File Structure
It will main control by Webserv which store each