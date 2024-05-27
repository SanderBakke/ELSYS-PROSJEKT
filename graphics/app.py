import webbrowser
import http.server
import socketserver
import threading
import subprocess

PORT = 8040
# DIRECTORY = '/home/elsys/Documents/graphics/index.html'
DIRECTORY = r"C:\Users\sande\Documents\graphics"

Handler = http.server.SimpleHTTPRequestHandler

class CalcThread(threading.Thread):
    def run(self):
        subprocess.call(['python', 'calculate.py'])

def main():
    print(f"Serving {DIRECTORY} at http://localhost:{PORT}")
    webbrowser.open(f'http://localhost:{PORT}/{DIRECTORY}/index.html')

    # Start the num_generator script in a separate thread
    calc = CalcThread()
    calc.start()

    # Start the HTTP server in a separate thread
    httpd = socketserver.TCPServer(("", PORT), Handler)
    httpd_thread = threading.Thread(target=httpd.serve_forever)
    httpd_thread.start()

    # Wait for both threads to finish
    calc.join()
    httpd_thread.join()

    httpd.server_close()

if __name__ == '__main__':
    main()
