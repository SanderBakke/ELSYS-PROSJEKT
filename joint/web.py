"""
Sever classes used in the web method
"""

from cgitb import html
import io
import glob
import json
import logging
import os
from pickle import FALSE, TRUE
import socketserver
import time
from http import server
import threading
from threading import Thread, Condition
from multiprocessing import Process
from utils import server_ip, detect_pi, warning
from flask import Flask, render_template
import subprocess


app = Flask(__name__)

@app.route('/index.html')
def index():
  return render_template('index.html')


if detect_pi():
    import picamera


class StreamingOutput(object):
    """Defines output for the picamera, used by request server"""

    def __init__(self):
        self.frame = None
        self.buffer = io.BytesIO()
        self.condition = Condition()
        self.count = 0

    def write(self, buf):
        if buf.startswith(b'\xff\xd8'):
            # New frame, copy the existing buffer's content and notify all
            # clients it's available
            self.buffer.truncate()
            with self.condition:
                self.frame = self.buffer.getvalue()
                self.condition.notify_all()
            self.buffer.seek(0)
            self.count += 1
        return self.buffer.write(buf)


class RequestHandler(server.BaseHTTPRequestHandler):
    """Request server, handles request from the browser"""
    output = None
    base_folder = None
    index_file = None
    custom_response = None

    def do_GET(self):
        if self.path == '/':
            self.redirect('/index.html', redir_type=301)
        elif self.path == '/stream.mjpg':
            self.serve_stream()
        elif self.path.startswith('/http') or self.path.startswith('/www'):
            self.redirect(self.path[1:])
        elif self.path.startswith('/stop'):
            self.send_response(200)
            self.end_headers()    
        # elif self.path.startswith('/camera'):
        #     self.redirect('/index2.html', redir_type=301)
 

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

    def send_404(self):
        self.send_error(404)
        self.end_headers()

    
    def serve_stream(self):
        self.send_response(200)
        self.send_header('Age', 0)
        self.send_header('Cache-Control', 'no-cache, private')
        self.send_header('Pragma', 'no-cache')
        self.send_header('Content-Type',
                         'multipart/x-mixed-replace; boundary=FRAME')
        self.end_headers()
        try:
            while True:
                with self.output.condition:
                    self.output.condition.wait()
                    frame = self.output.frame
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

    def log_message(self, format, *args):
        return


class WebpageServer(socketserver.ThreadingMixIn, server.HTTPServer):
    """Threaded HTTP server, forwards request to the RequestHandlerClass"""
    allow_reuse_address = True
    daemon_threads = True

    def __init__(self, server_address, RequestHandlerClass, stream_output,
                 index_file=None, debug=False,
                 custom_response=None):
        self.start = time.time()
        self.debug = debug
        RequestHandlerClass.output = stream_output
        RequestHandlerClass.base_folder = os.path.abspath(
            os.path.dirname(index_file))
        RequestHandlerClass.index_file = index_file
        RequestHandlerClass.custom_response = custom_response
        super(WebpageServer, self).__init__(server_address,
                                            RequestHandlerClass)


    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        print('Shutting down http server')
        if self.debug:
            finish = time.time()
            frame_count = self.RequestHandlerClass.output.count
            print('Sent {} images in {:.1f} seconds at {:.2f} fps'
                  .format(frame_count,
                          finish - self.start,
                          frame_count / (finish - self.start)))

class CThread(Thread):
    def run(self):
        subprocess.call(['python', 'get.py'])
def start_http_server(video_resolution, fps, server_port, index_file,
                      debug=False, custom_response=None):
    if debug:
        print('Using {} @ {} fps'.format(video_resolution, fps))

    with picamera.PiCamera(resolution=video_resolution,
                           framerate=fps) as camera:

        stream_output = StreamingOutput()
        s = WebpageServer(server_address=('', server_port),
                               RequestHandlerClass=RequestHandler,
                               stream_output=stream_output,
                               debug=debug,
                               index_file=index_file,
                               custom_response=custom_response)
        server_thread = Thread(target=s.serve_forever, daemon=True)
        camera.vflip = True
        camera.hflip = True
        dataThread = CThread()
        camera.start_recording(stream_output, format='mjpeg')

        try:            
            print('Visit the webpage at {}'.format(server_ip(server_port)))
            dataThread.start()
            server_thread.start()
            while True:
                time.sleep(0.1)
                # camera.capture('/home/teknostart/teknobil2022/projectfolder/image.jpg', use_video_port=True, splitter_port=2)
                
        finally:
            print('closing web server')
            camera.stop_recording()