THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
MODULES += $(THIS_DIR)
