#!/usr/bin/python3

import cgi # alert warning drpricated

form = cgi.FieldStorage()

print('Content-Type: text/html\r\n\r\n', end='')

print ('<html>')

print ('<head>')
print ('<title>Python CGI Script</title>')
print ('</head>')

print ('<body>')

print (f'<h1>Hi {form.getvalue("firstname")} {form.getvalue("lastname")}</h3>')
print ('<p>This is a Python CGI</p>')

print ('</body>')

print ('</html>')
