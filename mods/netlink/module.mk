ifeq ($(NETLINK_INCLUDED),)
NETLINK_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
netlink_DIRS := $(THIS_DIR)
netlink_CXXFLAGS := -Wno-deprecated-register
INCLUDES += $(THIS_DIR)

MODULES += netlink

endif