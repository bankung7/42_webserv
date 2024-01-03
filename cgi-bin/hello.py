#!/usr/bin/python3

import cgi # alert warning drpricated
import os
import sys

# get method
req_method = os.environ.get("REQUEST_METHOD", "")

# for key, value in os.environ.items():
#     print(f"{key}: {value}")

print ("Content-Type: text/html\r\n\r\n", end='')
print ('<html>')
print ('<head>')
print ('<title>Hello Word - First CGI Program</title>')

# if GET data
if req_method == "GET":
    query_string = os.environ.get("QUERY_STRING", "")
    param_dict = dict()
    param = query_string.split("&")
    
    for item in param:
        attr = item.split("=")
        param_dict[attr[0]] = attr[1]
    
    print (f'<h3>Hi {param_dict["firstname"]} {param_dict["lastname"]}</h3>')
    print ('<p>This is from GET Method</p>')
    
    
elif req_method == "POST":

    # Create instance of FieldStorage
    form = cgi.FieldStorage()

    # # Get data from fields
    first_name = form.getvalue('firstname')
    last_name = form.getvalue('lastname')

    # print (f'{post_data}')
    print (f'<h3>Hi {first_name} {last_name}</h3>')
    print ('<p>This is from POST Method</p>')

    
print ('</head>')
print ('<body>')
# print (f'<h2>Hello Mr.{first_name} {last_name}</h2>')
print ('</body>')
print ('</html>')