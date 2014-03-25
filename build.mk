BUILD_MK_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

ifeq ($(CC),)
 CC := gcc
endif

ifeq ($(CC),cc)
 CC := gcc
endif

ifeq ($(CXX),)
 CXX := g++
endif

ifndef CGC
CGC := cgc
endif
ifndef XXD
XXD := xxd
endif

ifndef ANT
ANT := ant
endif

ifndef AR
AR := ar
endif

ifndef RANLIB
RANLIB=ranlib
endif

ifdef USE_CCACHE
CCACHE=ccache
endif

CGC_PATH := $(realpath $(shell which $(CGC)))

ifndef TARGETDIR
TARGETDIR := $(CURDIR)/
endif


AS := $(PREFIX)as
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump

CC := $(PREFIX)$(CC)$(C_VERSION)
CXX := $(PREFIX)$(CXX)$(C_VERSION)
LD := $(CXX)

# Reset default src patterns if empty
ifeq ($(SRC_PATTERNS),)
  SRC_PATTERNS := .cpp .cxx .cc .c .s .glsl
endif

ifeq ($(HOST),android)
	include $(BUILD_MK_DIR)android/build.mk
else ifeq ($(HOST),emscripten)
	ifneq ($(DATA_FILES),)
 		LDFLAGS += $(addprefix --preload-file ,$(DATA_FILES))
 	endif
	# Override compiler changes since nothing else really works
	# Note: Maybe move above CC assignment to support PREFIX
	CC := emcc
	CXX := em++
endif

CFLAGS += $(addprefix -I, $(sort $(realpath $(INCLUDES))))
CXXFLAGS += $(CFLAGS)
COMP_CXXFLAGS += $(COMP_CFLAGS)

# LOCAL_FILES <= Source files relative to current directory + SRCDIR
# FILES <= Source files with exact path
FILES := $(realpath $(FILES))
FILES += $(realpath $(addprefix $(SRCDIR), $(LOCAL_FILES)))
# FILES <= Now full path of all source files

# Create corresponding list of OBJS from FILES (but only files that end with a pattern in SRC_PATTERNS)
OBJS := $(foreach PAT,$(SRC_PATTERNS), $(patsubst %$(PAT),%.o, $(filter %$(PAT),$(FILES))) )

# Also add all source files inside DIRS directories
RP_DIRS := $(realpath $(DIRS))
RP_DIRS += $(realpath $(addprefix $(SRCDIR), $(LOCAL_DIRS)))


OBJS += $(foreach PAT,$(SRC_PATTERNS), $(patsubst %$(PAT),%.o, $(wildcard $(addsuffix /*$(PAT), $(RP_DIRS)))) )

# Since all paths in OBJS are absolute, we dont add a slash after OBJDIR
OBJDIR := $(OBJDIR)$(HOST)
OBJFILES += $(addprefix $(OBJDIR), $(OBJS))

REAL_TARGET := $(TARGET)
TARGET := $(TARGET_PRE)$(TARGET)

LIBMODS := $(addsuffix .a,$(MODULES))
LIBMODS := $(addprefix $(OBJDIR)/,$(LIBMODS))

start_rule: $(TARGETDIR) $(OBJFILES) $(LIBMODS) $(TARGETDIR)$(TARGET)$(TARGET_EXT)

remove_target:
	rm -f $(TARGETDIR)$(TARGET)$(TARGET_EXT)

relink: remove_target start_rule
linkrun: remove_target run

test :

##
## MODULE RULE - Dynamic target and build rules for each module
##
define MODULE_template
$(1)_FILES := $$(realpath $($(1)_FILES))
$(1)_DIRS := $$(realpath $($(1)_DIRS))
$(1)_OBJS := $$(foreach PAT,$$(SRC_PATTERNS), $$(patsubst %$$(PAT),%.o, $$(filter %$$(PAT),$$($(1)_FILES))) )
$(1)_OBJS += $$(foreach PAT,$$(SRC_PATTERNS), $$(patsubst %$$(PAT),%.o, $$(wildcard $$(addsuffix /*$$(PAT), $$($(1)_DIRS)) )))
$(1)_OBJDIR := $$(OBJDIR)
$(1)_OBJS := $$(addprefix $$($(1)_OBJDIR), $$($(1)_OBJS))

$(1)_CFLAGS += $$(addprefix -I, $$(sort $$(realpath $$($(1)_INCLUDES))))
$(1)_CXXFLAGS += $$($(1)_CFLAGS)

$$(OBJDIR)/$(1).a : CXXFLAGS := $$($(1)_CXXFLAGS) $$(CXXFLAGS)
$$(OBJDIR)/$(1).a : CFLAGS := $$($(1)_CFLAGS) $$(CFLAGS)

ifneq ($$($(1)_CC),cc)
$$(OBJDIR)/$(1).a : CC := $$(PREFIX)$$($(1)_CC)
endif

$$(OBJDIR)/$(1).a : $$($(1)_OBJS)
	rm -f $$(OBJDIR)/$(1).a
	$$(AR) r $$(OBJDIR)/$(1).a $$($(1)_OBJS)

build_$(1) : $$(OBJDIR)/$(1).a

clean_$(1) :
	rm -f $$(OBJDIR)/$(1).a
	rm -f $$($(1)_OBJS)

rebuild_$(1) : clean_$(1) build_$(1)

#-include $$($(1)_OBJS:.o=.d)

$(OBJDIR)%.d: %.c
	@mkdir -p $(@D)
	@$(CC) -MM -MG  -MT '$(OBJDIR)$*.o' $(CFLAGS) $< > $@

$(OBJDIR)%.d: %.cpp
	@mkdir -p $(@D)
	$(CXX) -MM -MG -MT '$(OBJDIR)$*.o' $(CXXFLAGS) $< > $@
endef
## Generate the rules
$(foreach mod,$(MODULES),$(eval $(call MODULE_template,$(mod))))
##
## END MODULES
##


## Dependency files
-include $(OBJFILES:.o=.d)


# Implicit rules

$(OBJDIR)%.o: %.c
	@mkdir -p $(@D)
	$(CCACHE) $(CC) -c -MMD $(CFLAGS) $(COMP_CFLAGS) $< -o $@

$(OBJDIR)%.o: %.cpp
	@mkdir -p $(@D)
	$(CCACHE) $(CXX) -c -MMD $(CXXFLAGS) $(COMP_CXXFLAGS) $< -o $@

$(OBJDIR)%.o: %.cc
	@mkdir -p $(@D)
	$(CCACHE) $(CXX) -c $(CXXFLAGS) $(COMP_CXXFLAGS) $< -o $@

$(OBJDIR)%.o: %.cxx
	@mkdir -p $(@D)
	$(CCACHE) $(CXX) -c $(CXXFLAGS) $(COMP_CXXFLAGS) $< -o $@

$(OBJDIR)%.o: %.S
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)%.o: %.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)%_v.o: %_v.glsl
	@mkdir -p $(@D)
	if [ -n "$(CGC_PATH)" ] ; then $(CGC) -noentry -oglsl -profile vs_2_0 $< ; fi
	@mkdir -p .shader
	@cp $< .shader/$(<F)
	@$(XXD) -i .shader/$(<F) .shader/$(<F).cpp
	@$(CXX) -O2 -c .shader/$(<F).cpp -o $@

$(OBJDIR)%_f.o: %_f.glsl
	@mkdir -p $(@D)
	if [ -n "$(CGC_PATH)" ] ; then $(CGC) -noentry -oglsl -profile ps_2_0 $<  ; fi
	@mkdir -p .shader
	@cp $< .shader/$(<F)
	@$(XXD) -i .shader/$(<F) .shader/$(<F).cpp
	@$(CXX) -O2 -c .shader/$(<F).cpp -o $@

$(OBJDIR)%.d: %.c
	@mkdir -p $(@D)
	@$(CC) -MM -MG  -MT '$(OBJDIR)$*.o' $(CFLAGS) $< > $@

$(OBJDIR)%.d: %.cpp
	@mkdir -p $(@D)
	$(CXX) -MM -MG -MT '$(OBJDIR)$*.o' $(CXXFLAGS) $< > $@

$(OBJDIR)%.d: %.cc
	@mkdir -p $(@D)
	$(CXX) -MM -MG -MT '$(OBJDIR)$*.o' $(CXXFLAGS) $< > $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

$(OBJDIR)icon.o: icon.rc
	windres $< -O coff -o $@
	
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(TARGETDIR):
	mkdir -p $(TARGETDIR)

$(TARGETDIR)$(TARGET).a: $(TARGETDIR) $(OBJFILES) $(DEPS)
	rm -rf $(TARGETDIR)$(TARGET).a
	$(AR) r $(TARGETDIR)$(TARGET).a $(OBJFILES)
	$(RANLIB) $(TARGETDIR)$(TARGET).a
	 
$(TARGETDIR)$(TARGET): $(OBJFILES) $(LIBMODS) $(DEPS)
	$(LD) -o $(TARGETDIR)$(TARGET) $(LDFLAGS) $(OBJFILES) $(LIBMODS) $(LIBS)

$(TARGETDIR)$(TARGET).so: $(OBJFILES) $(LIBMODS) $(DEPS)
	$(LD) $(LDFLAGS) -Wl,-soname,$(TARGET).so -shared -o $(TARGETDIR)$(TARGET).so $(OBJFILES) $(LIBMODS) $(LIBS)

$(ANDROID_PROJECT) :
	android create project -p $(ANDROID_PROJECT) -n $(APP_NAME) -a ChangeMe -k $(ANDROID_PACKAGE) -t android-$(ANDROID_SDK_VERSION)
	rm -rf $(ANDROID_PROJECT)/src/*
	sed -i 's/ChangeMe/$(APP_NAME)/' $(ANDROID_PROJECT)/res/values/strings.xml

android_data : $(DATA_FILES)
	mkdir -p $(ANDROID_PROJECT)/assets/data
	cp -af $(DATA_FILES) $(ANDROID_PROJECT)/assets/data


$(TARGETDIR)$(TARGET).apk: $(ANDROID_PROJECT) $(TARGETDIR)$(TARGET).so
	mkdir -p $(ANDROID_PROJECT)/libs/armeabi
	cp $(TARGETDIR)$(TARGET).so $(ANDROID_PROJECT)/libs/armeabi/
	if [ -n "$(DATA_FILES)" ] ; then mkdir -p $(ANDROID_PROJECT)/assets/data ; cp -af $(DATA_FILES) $(ANDROID_PROJECT)/assets/data ; fi
	$(FIX_MANIFEST) $(ANDROID_PROJECT)/AndroidManifest.xml $(REAL_TARGET) $(ANDROID_SDK_VERSION) $(APP_NAME) $(ANDROID_PACKAGE)
	cd $(ANDROID_PROJECT) ; ANDROID_HOME=$(ANDROID_SDK) $(ANT) debug

$(TARGETDIR)$(TARGET).dll: $(OBJFILES) $(DEPS)
	$(LD) $(LDFLAGS) -shared -o $(TARGETDIR)$(TARGET).dll $(OBJFILES) $(LIBS)

$(TARGETDIR)$(TARGET).html: $(OBJFILES) $(LIBMODS) $(DEPS)
	$(LD) -O2 $(LDFLAGS) $(OBJFILES) $(LIBMODS) $(LIBS) -o $(TARGETDIR)$(TARGET).html

$(TARGETDIR)$(TARGET).js: $(OBJFILES) $(LIBMODS) $(DEPS)
	$(LD) -O2 $(LDFLAGS) $(OBJFILES) $(LIBMODS) $(LIBS) -o $(TARGETDIR)$(TARGET).js

## OBSOLETE ?

$(TARGETDIR)$(TARGET).elf: $(OBJFILES) $(DEPS)
	$(LD) $(LDFLAGS) $(OBJFILES) $(LIBS) -o $(TARGETDIR)$(TARGET).elf

$(TARGETDIR)$(TARGET).exe: $(OBJFILES) $(DEPS)
	$(LD) $(LDFLAGS) $(OBJFILES) $(LIBS) -o $(TARGETDIR)$(TARGET).exe

$(TARGETDIR)$(TARGET).bin: $(TARGETDIR)$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

#%/module.mk : %
#	mkdir -p %
#	git clone $(1) $$(BASENAME_$(2))

##

clean:
	rm -rf $(OBJDIR) $(TARGETDIR)$(TARGET)$(TARGET_EXT)
	
cleandep:
	rm -f $(OBJFILES:.o=.d)

superclean:
	rm -rf $(OBJDIR) $(TARGETDIR)$(TARGET)$(TARGET_EXT) $(addsuffix /*~, $(DIRS)) *.elf *~

run: start_rule
	$(realpath $(TARGETDIR))/$(TARGET)$(TARGET_EXT)
