import subprocess
import time
import xml.etree.ElementTree as ET


#make XML format
root = ET.Element("root")
angle = ET.SubElement(root, "angle")
angle.text = str(0)

# Compile the C program
while True:

    x = 0
    subprocess.run(['gcc', 'dummy.c', '-o', 'dummy'])

    # Run the C program and capture its output
    result = subprocess.check_output(['./dummy'])

    # Decode the byte string output and convert it to an integer
    x = int(result.decode())
    angle.text = str(x)

    tree = ET.ElementTree(root)
    #write to XML
    with open('angle.xml', 'wb') as f:
        tree.write(f)
    # time.sleep(1)

