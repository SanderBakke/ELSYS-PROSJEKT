import webbrowser
import http.server
import socketserver
import threading
import subprocess
import picamera
from io import BytesIO


PORT = 8010
DIRECTORY = '/home/elsys/Documents/video'
# DIRECTORY = r'C:\Users\sande\Documents\video'

Handler = http.server.SimpleHTTPRequestHandler

camera = picamera.PiCamera()
camera.resolution = (640, 480)
camera.framerate = 24
camera.rotation = 180

# Define a function to generate the video stream
def generate():
    stream = BytesIO()
    for _ in camera.capture_continuous(stream, 'jpeg', use_video_port=True):
        stream.seek(0) 
        yield b'--frame\r\nContent-Type: image/jpeg\r\n\r\n' + stream.read() + b'\r\n'
        stream.seek(0)
        stream.truncate()

class CThread(threading.Thread):
    def run(self):
        subprocess.call(['python', 'get.py'])

        
# Define a custom handler for the video stream URL
class VideoStreamHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/video.mjpg':
            self.send_response(200)
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=frame')
            self.end_headers()
            for frame in generate():
                self.wfile.write(frame)
        else:
            super().do_GET()

def main():
    print(f"Serving {DIRECTORY} at http://localhost:{PORT}")
    webbrowser.open(f'http://localhost:{PORT}/{DIRECTORY}/index.html')

    # Start the num_generator script in a separate thread
    dataThread = CThread()
    dataThread.start()

    # Start the HTTP server in a separate thread
    httpd = socketserver.TCPServer(("", PORT), Handler)
    httpd_thread = threading.Thread(target=httpd.serve_forever)
    httpd_thread.start()

    # Wait for both threads to finish
    dataThread.join()
    httpd_thread.join()

    httpd.server_close()

    # with socketserver.TCPServer(("", PORT), Handler) as httpd:
    #     try:
    #         httpd.RequestHandlerClass = VideoStreamHandler
    #         httpd.serve_forever()
    #     except KeyboardInterrupt:
    #         pass
    #     httpd.server_close()

if __name__ == '__main__':
    main()
