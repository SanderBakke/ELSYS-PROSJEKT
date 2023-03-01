from doctest import FAIL_FAST
import os
from pickle import FALSE
import subprocess
from tkinter import LEFT, RIGHT
from turtle import down
import Pyro4
from core import WebMethod

web_method = WebMethod(
    index_file=os.path.join(os.path.dirname(__file__), 'index.html')
)

web_method.serve()