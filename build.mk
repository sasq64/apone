# Reset default src patterns if empty
ifeq ($(SRC_PATTERNS),)
SRC_PATTERNS := .cpp .cxx .cc .c .s .glsl
endif

CFLAGS += $(addprefix -I, $(INCLUDES))
CXXFLAGS += $(CFLAGS)
COMP_CXXFLAGS += $(COMP_CFLAGS)
# LOCAL_FILES <= Source files relative to current directory + SRCDIR
# FILES <= Source files with exact path
FILES := $(realpath $(FILES))
FILES += $(realpath $(addprefix $(SRCDIR), $(LOCAL_FILES)))
# FILES <= Now full path of all source files

# Create corresponding list of OBJS from FILES
OBJS := $(foreach PAT,$(SRC_PATTERNS), $(patsubst %$(PAT),%.o, $(filter %$(PAT),$(FILES))) )

# Also add all source files inside MODULES directories
RP_MODULES := $(realpath $(MODULES))
OBJS += $(foreach PAT,$(SRC_PATTERNS), $(patsubst %$(PAT),%.o, $(wildcard $(addsuffix /*$(PAT), $(RP_MODULES)))) )

# Since all paths in OBJS are absolute, we dont add a slash after OBJDIR
OBJDIR := $(OBJDIR)$(HOST)
OBJFILES += $(addprefix $(OBJDIR), $(OBJS))

TARGET := $(TARGET_PRE)$(TARGET)

start_rule: $(TARGETDIR) $(OBJFILES) $(TARGETDIR)$(TARGET)$(TARGET_EXT)

remove_target:
	rm -f $(TARGETDIR)$(TARGET)$(TARGET_EXT)

relink: remove_target start_rule
linkrun: remove_target run

-include $(OBJFILES:.o=.d)

# Implicit rules

$(OBJDIR)%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c -MMD $(CFLAGS) $(COMP_CFLAGS) $< -o $@

$(OBJDIR)%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) -c -MMD $(CXXFLAGS) $(COMP_CXXFLAGS) $< -o $@

$(OBJDIR)%.o: %.cc
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) $(COMP_CXXFLAGS) $< -o $@

$(OBJDIR)%.o: %.cxx
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) $(COMP_CXXFLAGS) $< -o $@

$(OBJDIR)%.o: %.S
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)%.o: %.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)%_v.o: %_v.glsl
	@mkdir -p $(@D)
	$(CGC) -noentry -oglsl -profile vs_2_0 $< 
	@mkdir -p .shader
	@cp $< .shader/$(<F)
	@$(XXD) -i .shader/$(<F) .shader/$(<F).cpp
	@$(CXX) -O2 -c .shader/$(<F).cpp -o $@

$(OBJDIR)%_f.o: %_f.glsl
	@mkdir -p $(@D)
	$(CGC) -noentry -oglsl -profile ps_2_0 $< 
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
	$(AR) r $(TARGETDIR)$(TARGET).a $(OBJFILES)
	#$(RANLIB) $(TARGETDIR)$(TARGET).a
	 
$(TARGETDIR)$(TARGET).elf: $(OBJFILES) $(DEPS)
	$(LD) $(LDFLAGS) $(OBJFILES) $(LIBS) -o $(TARGETDIR)$(TARGET).elf

$(TARGETDIR)$(TARGET).exe: $(OBJFILES) $(DEPS)
	$(LD) $(LDFLAGS) $(OBJFILES) $(LIBS) -o $(TARGETDIR)$(TARGET).exe

$(TARGETDIR)$(TARGET): $(OBJFILES) $(DEPS)
	$(LD) -o $(TARGETDIR)$(TARGET) $(LDFLAGS) $(OBJFILES) $(LIBS) 

$(TARGETDIR)$(TARGET).bin: $(TARGETDIR)$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGETDIR)$(TARGET).so: $(OBJFILES) $(DEPS)
	$(LD) $(LDFLAGS) -Wl,-soname,$(TARGET).so -shared -o $(TARGETDIR)$(TARGET).so $(OBJFILES) $(LIBS)

$(TARGETDIR)$(TARGET).dll: $(OBJFILES) $(DEPS)
	$(LD) $(LDFLAGS) -shared -o $(TARGETDIR)$(TARGET).dll $(OBJFILES) $(LIBS)

$(TARGETDIR)$(TARGET).html: $(OBJFILES) $(DEPS)
	$(LD) -O2 $(LDFLAGS) $(LIBS) $(OBJFILES) -o $(TARGETDIR)$(TARGET).html

$(TARGETDIR)$(TARGET).js: $(OBJFILES) $(DEPS)
	$(LD) -O2 $(LDFLAGS) $(LIBS) $(OBJFILES) -o $(TARGETDIR)$(TARGET).js

clean:
	rm -rf $(OBJDIR) $(TARGETDIR)$(TARGET)$(TARGET_EXT)
	
cleandep:
	rm -f $(OBJFILES:.o=.d)

superclean:
	rm -rf $(OBJDIR) $(TARGETDIR)$(TARGET)$(TARGET_EXT) $(addsuffix /*~, $(MODULES)) *.elf *~

run: start_rule
	$(CURDIR)/$(TARGET)$(TARGET_EXT)

