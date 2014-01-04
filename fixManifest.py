#!/usr/bin/python
import sys
import os
import os.path
from xml.dom.minidom import parseString

def main(argv) :
	libname = argv[1]
	docText = open(argv[0]).read()
	doc = parseString(docText)
	meta = doc.getElementsByTagName('meta-data')
	for tag in meta :
		if tag.getAttribute('android:name') == 'android.app.lib_name' :
			tag.setAttribute('android:value', libname)
			f = open(argv[0], 'w')
			f.write(doc.toxml())
			f.close()
			print "Wrote file"
			return
	print "Could not find tag"

if __name__ == "__main__":
	main(sys.argv[1:])
