include ../config.mk

include module.mk

CFLAGS += -DUNIT_TEST
TARGET := test
LOCAL_FILES += 
CFLAGS += -I$(realpath ../test)
FILES += $(realpath ../test/catch.cpp)

define TESTMOD_template
FILES += $$(realpath $($(1)_FILES))
DIRS += $$(realpath $($(1)_DIRS))
endef
## Generate the rules
$(foreach mod,$(MODULES),$(eval $(call TESTMOD_template,$(mod))))

MODULES :=

include ../build.mk