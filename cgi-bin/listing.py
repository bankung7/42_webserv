#!/use/bin/python

import sys # for argv
import os # for path
import cgi

def main():

    path_info = os.environ["PATH_INFO"]
    query_string = os.environ["QUERY_STRING"]

    print("HTTP/1.1 200 OK\r\n", end='')
    print("Content-Type: text/html\r\n\r\n", end='')


    print("<html>")
    print("<head><title>CGI testing</title></head>")
    print("<body>")

    print("<h1>Hello, form the CGI</h1>")
    print(f"<p>The path info is : {path_info}")
    print(f"<p>The query string is : {query_string}")

    print("</body>")
    print("</html>")


main()