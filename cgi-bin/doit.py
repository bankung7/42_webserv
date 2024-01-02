import os

def error(array):
    print("HTTP/1.1 400 Bad Request\r\n", end='')
    print("Content-Type: text/html\r\n\r\n", end='')
    print("404 : BAD REQUEST")

def doit():
    input = os.environ["QUERY_STRING"]
    array = input.split('&')

    if len(array) != 3:
        return error(array)

    lprefix = array[0][7:]
    if "prefix=" != array[0][:7]:
        return error(array)

    firstname = array[1][10:]
    if "firstname=" != array[1][:10]:
        return error(array)
    
    lastname = array[2][9:]
    if "lastname=" != array[2][:9]:
        return error(array)

    match lprefix:
        case "0":
            prefix = "Private"
        case "1":
            prefix = "Sergeant"
        case "2":
            prefix = "Captain"
        case "3":
            prefix = "Colonel"
        case "4":
            prefix = "General"
        case _:
            prefix = "Unknown"

    print("HTTP/1.1 200 OK\r\n", end='')
    print("Content-Type: text/html\r\n\r\n", end='')


    print("<html>")
    print("<head><title>CGI testing</title></head>")
    print("<body>")

    print("<h1>Hello, form the CGI</h1>")
    print(f"<p>Rank is [{prefix}]")
    print(f"<p>Firstname is [{firstname}]")
    print(f"<p>Lastname is [{lastname}]")

    print("<p>Attention!!! Goodbye</p>")

    print("</body>")
    print("</html>")

doit()