import pinger
import http.server
import socketserver
import threading

# create server
# every 10 seconds call pinger
# refresh graph

PORT = 8000

def main():
    Handler = http.server.SimpleHTTPRequestHandler
    p = pinger.Pinger()
    ping_thread = threading.Thread(target=p.multiple_ping)
    ping_thread.start()
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        print("serving at port", PORT)
        httpd.serve_forever()

if __name__ == '__main__':
    main()
