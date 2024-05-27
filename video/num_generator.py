import xml.etree.ElementTree as ET
import random
import time

start = 0

# Generate new data every second and append it to the XML file
while True:
    
    root = ET.Element("root")

    angle = ET.SubElement(root, "angle")
    
    angle.text = str(random.randint(1, 180))
        

    tree = ET.ElementTree(root)
    
    with open('angle.xml', 'wb') as f:
        tree.write(f)
    # start +=1
    time.sleep(1)
