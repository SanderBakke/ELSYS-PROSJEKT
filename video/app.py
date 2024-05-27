import webbrowser
import http.server
import socketserver
import threading
import subprocess

PORT = 8010
DIRECTORY = '/home/elsys/Documents/video'
# DIRECTORY = r'C:\Users\sande\Documents\video'

Handler = http.server.SimpleHTTPRequestHandler

class CThread(threading.Thread):
    def run(self):
        subprocess.call(['python', 'get.py'])

        

def main():
    print(f"Serving {DIRECTORY} at http://localhost:{PORT}")
    webbrowser.open(f'http://localhost:{PORT}/{DIRECTORY}/index.html')

    # Start the num_generator script in a separate thread
    dataThread = CThread()
    dataThread.start()

    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            pass
        httpd.server_close()

if __name__ == '__main__':
    main()
