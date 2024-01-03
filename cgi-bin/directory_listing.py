#!/usr/local/bin/python

import sys # for argv
import os # for path
from datetime import datetime

def main():

    path_info = os.environ["PATH_INFO"]
    query_string = os.environ["QUERY_STRING"][1:]

    location = query_string[len(path_info):]
    index = len(location) - location.rfind("/", 0, len(location))
    parent_location = location[:index]
    
    os.chdir(query_string) # goto that directory
    list_dir = list(os.listdir("./")) # list the current path
    
    list_dic = dict()
    for item in list_dir:
        stat = os.stat(item)
        list_dic[item] = stat

    print("HTTP/1.1 200 OK\r\n", end='')
    print("Content-Type: text/html\r\n\r\n", end='')

    print("<html>")
    print("<head><title>CGI indexing</title></head>")
    print("<body>")

    print(f"<h1>Index of {os.getcwd()}</h1>")
    print(f"<h1>path_info {path_info}</h1>")
    print(f"<h1>query string of {query_string}</h1>")
    print(f"<h1>parent of {parent_location}</h1>")
    print(f"<h1>parent of {index}</h1>")
    print(f"<h1>Index of {location}</h1>")
    print(f"<div>")
    
    print(f"<table width=\"800px\">")
    
    print("<tr>")
    print("<th width=\"20%\">Name</th>")
    print("<th width=\"30%\">Last Modified</th>")
    print("<th width=\"20%\">Size</th>")
    print("<th width=\"30%\">Description</th>")
    print("</tr>")
    
    # parent dir
    print(f"<tr>")
    print(f"<td><a href=\"{parent_location}\">../</a></td>")
    print(f"</tr>")

    for item in list_dir:
        link = location + "/" + item
        print(f"<tr>")
        print(f"<td><a href=\"{link}\">{location} {link}</a></td>")
        print(f"<td>{datetime.fromtimestamp(list_dic[item].st_mtime).strftime('%d-%b-%Y %H:%M')}</td>")
        print(f"<td style=\"text-align:right;\">{list_dic[item].st_size} bytes</td>")
        print(f"<td>-</td>")
        print(f"</tr>")
        
    print(f"</table>")
    
    print(f"</div>")
    print("</body>")
    print("</html>")

main()