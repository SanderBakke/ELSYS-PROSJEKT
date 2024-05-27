import xml.etree.ElementTree as ET
import random
import time

start = 0

# Generate new data every second and append it to the XML file
while True:
    
    root = ET.Element("root")

    for i in range(1, 6):
        sector_n = ET.SubElement(root, "sektor" + str(i))
        # if (start<=15):
                # sector_n.text = str(0)
        # else:
        sector_n.text = str(random.randint(1, 10))
        

    tree = ET.ElementTree(root)
    
    with open('energy.xml', 'wb') as f:
        tree.write(f)
    # start +=1
    time.sleep(1)
