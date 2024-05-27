import webbrowser
import http.server
import socketserver
import threading
import subprocess
from http.server import HTTPServer, SimpleHTTPRequestHandler
from io import BytesIO
from picamera import PiCamera
import time

PORT = 8090
DIRECTORY = '/home/elsys/Documents/graphics'

Handler = http.server.SimpleHTTPRequestHandler

class CalcThread(threading.Thread):
    def run(self):
        subprocess.call(['python', 'calculate.py'])


# Start the num_generator script in a separate thread
calc = CalcThread()
calc.start()

    # with socketserver.TCPServer(("", PORT), Handler) as httpd:
    #     try:
    #         httpd.serve_forever()
    #     except KeyboardInterrupt:
    #         pass
    #     httpd.server_close()
    # Start the Pi Camera
camera = PiCamera()
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

# Set up the HTTP server to serve the index file and video stream
handler = SimpleHTTPRequestHandler
httpd = HTTPServer(('0.0.0.0', 8000), handler)

# Define a custom handler for the video stream URL
class VideoStreamHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/video.mjpg':
            self.send_response(200)
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=frame')
            self.end_headers()
            for frame in generate():
                self.wfile.write(frame)
        else:
            super().do_GET()

# Start the HTTP server
httpd.RequestHandlerClass = VideoStreamHandler
print('Server started on http://localhost:8000')
httpd.serve_forever()


# if __name__ == '__main__':
#     main()
