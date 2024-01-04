#!/usr/bin/python3

import os # for path
from datetime import datetime
import cgi

def main():
    
    form = cgi.FieldStorage()

    root = form.getvalue('root')
    url = form.getvalue('url')
    
    path = os.path.join(str(root), str(url))
    path = os.path.join("./", path)
    current_path = os.path.join(os.getcwd(), path[1:])
    
    if current_path[-1] == '/':
        current_path = current_path[:-1]
    
    os.chdir(path) # goto that directory
    list_dir = list(os.listdir("./")) # list the current path
    
    list_dict = dict()
    for item in list_dir:
        stat = os.stat(item)
        list_dict[item] = stat
        
    if url[-1] == '/':
        url = url[:-1]
        
    purl = "/" + url
    purl = purl[:purl.rfind("/") + 1]
    

    print('Content-Type: text/html\r\n\r\n', end='')

    print('<html>')
    print('<head><title>CGI indexing</title></head>')
    print('<body>')
    
    print(f'<h1>Index of {url}</h1>')
    
    print('<div>')
    print('<table width="800px">')
    print('<tr>')
    print('<th width="20%">Name</th>')
    print('<th width="30%">Last Modified</th>')
    print('<th width="20%">Size</th>')
    print('<th width="30%">Description</th>')
    print('</tr>')
    
    # parent dir
    print('<tr>')
    print(f'<td><a href="{purl}">../</a></td>')
    print('<td>-</td>')
    print('<td>-</td>')
    print('<td>-</td>')
    print('</tr>')

    for item in list_dir:
        print('<tr>')
        print(f'<td><a href="/{url}/{item}">{item}</a></td>')
        print(f'<td>{datetime.fromtimestamp(list_dict[item].st_mtime).strftime("%d-%b-%Y %H:%M")}</td>')
        print(f'<td style="text-align:right;">{list_dict[item].st_size} bytes</td>')
        print(f'<td>-</td>')
        print('</tr>')
        
    print('</table>')
    
    print('</div>')
    print('</body>')
    print('</html>')

main()