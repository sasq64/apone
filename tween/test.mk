TWEEN_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

$(warning $(TWEEN_DIR))

include $(TWEEN_DIR)../config.mk

include $(TWEEN_DIR)module.mk

CFLAGS += -DTWEEN_UNIT_TEST -pthread -g -O3
LDFLAGS += -pthread

ifeq ($(TARGET),)
TARGET := tweentest
endif

LOCAL_FILES += 
CFLAGS += -I$(realpath $(TWEEN_DIR)../test)
FILES += $(realpath $(TWEEN_DIR)../test/catch.cpp)

define TESTMOD_template
FILES += $$(realpath $($(1)_FILES))
DIRS += $$(realpath $($(1)_DIRS))
endef
## Generate the rules
$(foreach mod,$(MODULES),$(eval $(call TESTMOD_template,$(mod))))

MODULES :=

include $(TWEEN_DIR)/../build.mk