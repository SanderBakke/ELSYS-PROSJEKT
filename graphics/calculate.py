import xml.etree.ElementTree as ET
import time
import subprocess

# Compile the C program

while True:
    x = 0
    subprocess.run(['gcc', 'dummy.c', '-o', 'dummy'])

# Run the C program and capture its output
    result = subprocess.check_output(['./dummy'])

# Decode the byte string output and convert it to an integer
    x = int(result.decode())
    # print(x) 
    time.sleep(0.5)
    

    angle = x
    sector_division =180/5 ## Hvis radianer = math.pi/5
    growth_num =5

    #make XML format
    root = ET.Element("root")

    sector_1 = ET.SubElement(root, "sektor" + str(1))
    sector_2 = ET.SubElement(root, "sektor" + str(2))
    sector_3 = ET.SubElement(root, "sektor" + str(3))
    sector_4 = ET.SubElement(root, "sektor" + str(4))
    sector_5 = ET.SubElement(root, "sektor" + str(5))
    
    #Set everyting to 0
    sector_1.text = str(0)
    sector_2.text = str(0)
    sector_3.text = str(0)
    sector_4.text = str(0)
    sector_5.text = str(0)

    #Test whict sector is growing  and add to that one
    if (0 <= angle < sector_division):
        sector_1.text = str(growth_num)

    elif(sector_division <= angle < sector_division*2):
        sector_2.text = str(growth_num)

    elif(sector_division*2 <= angle < sector_division*3):
        sector_3.text = str(growth_num)

    elif(sector_division*3 <= angle < sector_division*4):
        sector_4.text = str(growth_num)

    elif(sector_division*4 <= angle <= sector_division*5):
        sector_5.text = str(growth_num)

    else:
        continue

    tree = ET.ElementTree(root)
    
    #write to XML
    with open('energy.xml', 'wb') as f:
        tree.write(f)

        
    time.sleep(1)


    