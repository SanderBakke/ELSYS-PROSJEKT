import webbrowser
import http.server
import socketserver
# import threading
import subprocess
import picamera
# from io import BytesIO
import os
import io
# import picamera
import logging
# import socketserver
from threading import Condition, Thread
from http import server
import socket
# import serial
import fcntl
import struct
import warnings

def server_ip(port):
    online_ips = []
    for interface in [b'eth0', b'wlan0']:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            ip = socket.inet_ntoa(fcntl.ioctl(
                sock.fileno(),
                0x8915,
                struct.pack('256s', interface[:15])
            )[20:24])
            online_ips.append(ip)
        except OSError:
            pass
        sock.close()
    return ' or '.join(['{}:{}'.format(ip, port) for ip in online_ips])



# DIRECTORY = '/home/elsys/Documents/video'
# DIRECTORY = r'C:\Users\sande\Documents\video'





class CThread(Thread):
    def run(self):
        subprocess.call(['python', 'get.py'])

class StreamingOutput(object):
    def __init__(self):
        self.frame = None
        self.buffer = io.BytesIO()
        self.condition = Condition()

    def write(self, buf):
        if buf.startswith(b'\xff\xd8'):
            # New frame, copy the existing buffer's content and notify all
            # clients it's available
            self.buffer.truncate()
            with self.condition:
                self.frame = self.buffer.getvalue()
                self.condition.notify_all()
            self.buffer.seek(0)
        return self.buffer.write(buf)
class StreamingHandler(server.BaseHTTPRequestHandler):
    output = None
    base_folder = None
    index_file = None
    custom_response = None

    def do_GET(self):
        if self.path == '/':
            self.send_response(301)
            self.send_header('Location', '/index.html')
            self.end_headers()
        elif self.path == '/index.html':
            try:
                with open('index.html', 'rb') as f:
                    content = f.read()
                    self.send_response(200)
                    self.send_header('Content-Type', 'text/html')
                    self.send_header('Content-Length', len(content))
                    self.end_headers()
                    self.wfile.write(content)
            except FileNotFoundError:
                self.send_error(404, 'File not found: index.html')
        elif self.path == '/stream.mjpg':
            self.send_response(200)
            self.send_header('Age', 0)
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=FRAME')
            self.end_headers()
            try:
                while True:
                    with output.condition:
                        output.condition.wait()
                        frame = output.frame
                    self.wfile.write(b'--FRAME\r\n')
                    self.send_header('Content-Type', 'image/jpeg')
                    self.send_header('Content-Length', len(frame))
                    self.end_headers()
                    self.wfile.write(frame)
                    self.wfile.write(b'\r\n')
            except Exception as e:
                logging.warning(
                    'Removed streaming client %s: %s',
                    self.client_address, str(e))
        else:
            path = os.path.join(self.base_folder, self.path[1:])
            if os.path.isfile(path):
                self.serve_path(path)
            elif self.custom_response:
                response = self.custom_response(self.path)
                if response:
                    if response.startswith('redirect='):
                        new_path = response[response.find('=') + 1:]
                        self.redirect(new_path)
                    else:
                        self.serve_content(response.encode('utf-8'))
                else:
                    warning(message='Bad response. {}. custom '
                                    'response function returned nothing'
                            .format(self.requestline), filter='default')
                    self.send_404()
            else:
                warning(message='Bad response. {}. Could not find {}'
                        .format(self.requestline, path), filter='default')
                self.send_404()

    def do_POST(self):
        self.send_404()

    def serve_content(self, content, content_type='text/html'):
        self.send_response(200)
        self.send_header('Content-Type', content_type)
        self.send_header('Content-Length', len(content))
        self.end_headers()
        self.wfile.write(content)

    def serve_path(self, path):
        if '.css' in path:
            content_type = 'text/css'
        elif '.js' in path:
            content_type = 'text/javascript'
        elif '.xml' in path:
            content_type = 'text/xml'
        else:
            content_type = 'text/html'
        with open(path, 'rb') as f:
            content = f.read()
        self.serve_content(content, content_type)

    def redirect(self, path, redir_type=302):
        self.send_response(redir_type)
        self.send_header('Location', path)
        self.end_headers()
    
    def warning(message, filter='error', category=UserWarning):
        warnings.simplefilter(filter, category)
        warnings.formatwarning = warning_format
        warnings.warn(message)

    def warning_format(message, category, filename, lineno,
                   file=None, line=None):
        return 'WARNING:\n  {}: {}\n  File: {}:{}\n'.format(
            category.__name__, message, filename, lineno)
    
        
    
 
class StreamingServer(socketserver.ThreadingMixIn, server.HTTPServer):
    allow_reuse_address = True
    daemon_threads = True

with picamera.PiCamera(resolution='640x480', framerate=24) as camera:
    output = StreamingOutput()
    #Uncomment the next line to change your Pi's Camera rotation (in degrees)
    #camera.rotation = 90
    camera.vflip = True
    camera.hflip = True
    # dataThread = CThread()
    # dataThread.start()
    camera.start_recording(output, format='mjpeg')

    try:
        # dataThread = CThread()
        # dataThread.start()
        address = ('', 8000)
        server = StreamingServer(address, StreamingHandler)
        print('Visit the webpage at {}'.format(server_ip(8000)))

        server.serve_forever()
    finally:
        print('closing web server')
        camera.stop_recording()

