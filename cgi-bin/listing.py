import sys # for argv
import os # for path

def main():

    path_info = os.environ["PATH_INFO"]
    query_string = os.environ["QUERY_STRING"][1:]
    
    current_path = os.getcwd()
    
    list_dir = list(os.listdir(query_string))
    
    # file_dic = dict()
    # for item in list_dir:
    #     stat = os.stat(item)
    #     file_dic[item] = stat

    print("HTTP/1.1 200 OK\r\n", end='')
    print("Content-Type: text/html\r\n\r\n", end='')

    print("<html>")
    print("<head><title>CGI indexing</title></head>")
    print("<body>")

    print(f"<h1>Index of {query_string}</h1>")
    print(f"<div>")
    
    print(f"<table width=\"500px\">")
    
    print("<tr>")
    print("<th width=\"50%\">Name</th>")
    print("<th width=\"50%\">Size</th>")
    print("</tr>")
    
    for item in list_dir:
        print(f"<tr>")
        print(f"<td>{item}</td>")
        print(f"<td>size</td>")
        print(f"</tr>")
        
    print(f"</table>")
    
    print(f"</div>")
    print("</body>")
    print("</html>")

main()