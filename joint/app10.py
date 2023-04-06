import os
import subprocess
from core import WebMethod



# Create the WebMethod class
web_method = WebMethod(
    index_file=os.path.join(os.path.dirname(__file__), 'index.html'))
# Start serving the web page, blocks the program after this point
web_method.serve()