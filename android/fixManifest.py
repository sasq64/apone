#!/usr/bin/python
import sys
import os
import os.path
from xml.dom.minidom import parseString

def tag(node, path, set = "", where = "", create = False, noclobber = False) :
	if not path :
		if set :
			if where :
				parts = where.split("=")
				if node.getAttribute(parts[0]) != parts[1] :
					print parts[0] + " does not match " + parts[1]
					return False
			parts = set.split("=")
			if noclobber and node.getAttribute(parts[0]) :
				print parts[0] + ' already set, wont clobber'
			node.setAttribute(parts[0], parts[1])
			return True
	else :
		parts = path.split(".");
		first = parts[0]
		rest = '.'.join(parts[1:])
		nodes = node.getElementsByTagName(first)
		if not nodes and not where :
			doc = node
			while doc.parentNode :
				doc = doc.parentNode
			nodes = [ node.appendChild(doc.createElement(first)) ]
		done = False
		for n in nodes :
			done |= tag(n, rest, set, where, create, noclobber);
		if not rest and not done and where and set and create :
			doc = node
			while doc.parentNode :
				doc = doc.parentNode
			node = node.appendChild(doc.createElement(first))
			parts = where.split("=")
			node.setAttribute(parts[0], parts[1])
			parts = set.split("=")
			node.setAttribute(parts[0], parts[1])
			done = True
		return done


def main(argv) :
	libname = argv[1]
	sdkver = argv[2]
	appname = argv[3]
	pkgname = argv[4]
	docText = open(argv[0]).read()
	doc = parseString(docText)
	#android:name="android.app.lib_name" android:value="demo"/
	tag(doc, 'manifest.uses-sdk', set='android:targetSdkVersion='+sdkver, noclobber=True)
	tag(doc, 'manifest.uses-feature', set='android:glEsVersion=0x00020000', noclobber=True)
	tag(doc, 'manifest.uses-feature', set='android:glEsVersion=0x00020000', noclobber=True)
	#tag(doc, 'manifest.application', set='android:hasCode=false', noclobber=True)
	tag(doc, 'manifest.application.activity', set='android:name=android.app.NativeActivity', where='android:name=ChangeMe')
	tag(doc, 'manifest.application.activity', set='android:configChanges=orientation|keyboardHidden', noclobber=True) 
	tag(doc, 'manifest.application.activity', set='android:screenOrientation=landscape', noclobber=True)
	tag(doc, 'manifest.application.activity', set='android:theme=@android:style/Theme.NoTitleBar.Fullscreen', noclobber=True)
	tag(doc, 'manifest.application.activity.meta-data', where='android:name=android.app.lib_name', set='android:value=' + libname, create=True)
	tag(doc, 'manifest.application.activity.intent-filter', set='android:name=' + appname, noclobber=True)
	f = open(argv[0], 'w')
	f.write(doc.toxml())
	f.close()
	print "Wrote file"

	# activity = doc.getElementsByTagName('activity')
	# meta = doc.getElementsByTagName('meta-data')
	# if !meta :
	# 	doc.appendChild(
	# for tag in meta :
	# 	if tag.getAttribute('android:name') == 'android.app.lib_name' :
	# 		tag.setAttribute('android:value', libname)
	# 		f = open(argv[0], 'w')
	# 		f.write(doc.toxml())
	# 		f.close()
	# 		print "Wrote file"
	# 		return
	# print "Could not find tag"

if __name__ == "__main__":
	main(sys.argv[1:])
