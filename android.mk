
all:
	HOST=android make
	cp libgrappix.so android/libs/armeabi
	(cd android ; ant debug)

install:
	(cd android ; ant installd) 
