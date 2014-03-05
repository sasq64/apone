

ANDROID TARGETS
---------------

ndk platform

This is the what the C++ compiler uses. It may be OK to always use latest target here, I am not sure.


project.properties

This is the jars that are actually linked in. Should be same as targetSDK

AndroidManifest.xml

targetSDK - see above
minSDK - minimal supported