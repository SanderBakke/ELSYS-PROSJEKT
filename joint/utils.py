"""
Different utility functions practical for ROV control
"""

import ctypes
from operator import truediv
import os
import platform
import signal
import socket
import struct
import subprocess
import warnings
import distro
from picamera import PiCamera

def detect_pi():
    #return platform.linux_distribution()[0].lower() == 'debian'
    return distro.linux_distribution()

if detect_pi():
    import serial
    import fcntl


def is_int(number):
    if isinstance(number, int):
        return True
    else:
        try:
            if isinstance(int(number), int):
                return True
        except ValueError:
            pass
    return False


def resolution_to_tuple(resolution):
    if 'x' not in resolution:
        raise ValueError('Resolution must be in format WIDTHxHEIGHT')
    screen_size = tuple([int(val) for val in resolution.split('x')])
    if len(screen_size) is not 2:
        raise ValueError('Error in parsing resolution, len is not 2')
    return screen_size


def preexec_function():
    signal.signal(signal.SIGINT, signal.SIG_IGN)


def valid_resolution(resolution):
    if 'x' in resolution:
        w, h = resolution.split('x')
        if is_int(w) and is_int(h):
            return resolution
    warning('Resolution must be WIDTHxHEIGHT')


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


def check_requirements():
    if detect_pi():
        #camera = subprocess.check_output(['vcgencmd','get_camera']).decode().rstrip()
        camera = PiCamera()
        return True
        
        # if '0' in camera:
        #     warning('Camera not enabled or connected properly')
        #     return False
        # else:
        #     return True
    else:
        warning('eduROV only works on a raspberry pi')
        return False


  


def warning(message, filter='error', category=UserWarning):
    warnings.simplefilter(filter, category)
    warnings.formatwarning = warning_format
    warnings.warn(message)


def warning_format(message, category, filename, lineno,
                   file=None, line=None):
    return 'WARNING:\n  {}: {}\n  File: {}:{}\n'.format(
        category.__name__, message, filename, lineno)


def free_drive_space(as_string=False):
    """
    Checks and returns the remaining free drive space

    Parameters
    ----------
    as_string : bool, optional
        set to True if you want the function to return a formatted string.
        4278 -> 4.28 GB

    Returns
    -------
    space : float or str
        the remaining MB in float or as string if *as_string=True*
    """
    if platform.system() == 'Windows':
        free_bytes = ctypes.c_ulonglong(0)
        ctypes.windll.kernel32.GetDiskFreeSpaceExW(ctypes.c_wchar_p('/'),
                                                   None, None,
                                                   ctypes.pointer(free_bytes))
        mb = free_bytes.value / 1024 / 1024
    else:
        st = os.statvfs('/')
        mb = st.f_bavail * st.f_frsize / 1024 / 1024

    if as_string:
        if mb >= 1000:
            return '{:.2f} GB'.format(mb / 1000)
        else:
            return '{:.0f} MB'.format(mb)
    else:
        return mb


def cpu_temperature():
    """
    Checks and returns the on board CPU temperature

    Returns
    -------
    temperature : float
        the temperature
    """
    cmds = ['/opt/vc/bin/vcgencmd', 'measure_temp']
    response = subprocess.check_output(cmds).decode()
    return float(response.split('=')[1].split("'")[0].rstrip())