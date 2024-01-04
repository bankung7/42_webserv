#!/usr/bin/python3

import cgi # alert warning drpricated

form = cgi.FieldStorage()

print ("Content-Type: text/html\r\n\r\n", end='')

print ('<html>')
print ('<head>')
print ('<title>Hello Word - First CGI Program</title>')
print (f'<h3>Hi {form.getvalue("firstname")} {form.getvalue("lastname")}</h3>')
print ('<p>This is from GET Method</p>')
print ('</head>')
print ('<body>')
print ('</body>')
print ('</html>')