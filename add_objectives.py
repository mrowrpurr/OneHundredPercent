import xml.etree.ElementTree as ET

# Filepath to the XML file
file_path = r"HazTheCompletionizt.esx"

# Number of quest objectives to add
total_qobjs = 500

# Parse the XML file
tree = ET.parse(file_path)
root = tree.getroot()

# Find the <QUST> element
qust = root.find(".//QUST")

# Find the last <QOBJ> element to determine the starting index
existing_qobjs = qust.findall("QOBJ")
current_count = len(existing_qobjs)
next_index = current_count

# Add new quest objectives up to the desired total
for i in range(next_index, total_qobjs):
    # Create and append <QOBJ>
    qobj = ET.Element("QOBJ")
    qobj.text = str(i)
    qust.append(qobj)

    # Create and append <FNAM>
    fnam = ET.Element("FNAM")
    fnam.text = "0"
    qust.append(fnam)

    # Create and append <NNAM>
    nnam = ET.Element("NNAM")
    nnam.text = ""
    qust.append(nnam)

# Write the updated XML back to the file
tree.write(file_path, encoding="utf-8", xml_declaration=True)

print(f"Added quest objectives up to {total_qobjs}.")
