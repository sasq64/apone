start_rule :
	+make -f hvsc.mk

.DEFAULT :
	+make -f hvsc.mk $@
