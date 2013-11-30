
INCLUDES := $(addprefix $(SRCDIR), $(INCLUDES))

SRC_INCLUDES := $(addprefix -I,$(INCLUDES))
CFLAGS += $(SRC_INCLUDES)

SRC_MODULES := $(realpath $(MODULES))

CXXFLAGS := $(CXXFLAGS) $(CFLAGS)

MODULEOBJS := $(patsubst %.cpp,%.o, $(wildcard $(addsuffix /*.cpp, $(SRC_MODULES))))
MODULEOBJS += $(patsubst %.cxx,%.o, $(wildcard $(addsuffix /*.cxx, $(SRC_MODULES))))
MODULEOBJS += $(patsubst %.cc,%.o, $(wildcard $(addsuffix /*.cc, $(SRC_MODULES))))
MODULEOBJS += $(patsubst %.c,%.o, $(wildcard $(addsuffix /*.c, $(SRC_MODULES))))
MODULEOBJS += $(patsubst %.s,%.o, $(wildcard $(addsuffix /*.s, $(SRC_MODULES))))

OBJDIR := $(OBJDIR)$(HOST)/

OBJFILES := $(addprefix $(OBJDIR),$(MODULEOBJS))

ifneq ($(ACTIVEFILE),)
 ifneq ($(findstring $(ACTIVEFILE),$(MAIN_FILES)),)
  MAINOBJ := $(patsubst %.cpp,%.o,$(ACTIVEFILE))
 endif
endif

OBJS := $(addprefix $(SRCDIR), $(MAINOBJ) $(OBJS))

#OBJFILES += $(addprefix $(OBJDIR), $(subst ../,,$(OBJS)))
OBJFILES += $(addprefix $(OBJDIR), $(OBJS))

DEPS := $(XDEPENDS) $(DEPS)

TARGET := $(TARGET_PRE)$(TARGET)

start_rule: $(TARGETDIR) $(OBJFILES) $(TARGETDIR)$(TARGET)$(TARGET_EXT)

remove_target:
	rm -f $(TARGETDIR)$(TARGET)$(TARGET_EXT)

relink: remove_target start_rule

linkrun: remove_target run
relink: remove_target start_rule


-include $(OBJFILES:.o=.d)

# Implicit rules

$(OBJDIR)%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c -MMD $(CFLAGS) $< -o $@

$(OBJDIR)%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) -c -MMD $(CXXFLAGS) $< -o $@

$(OBJDIR)%.o: %.cc
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJDIR)%.o: %.cxx
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJDIR)%.o: %.S
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)%.o: %.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)%.d: %.c
	@mkdir -p $(@D)
	@$(CC) -MM -MG  -MT '$(OBJDIR)$*.o' $(CFLAGS) $< > $@

$(OBJDIR)%_v.o: %_v.glsl
	@mkdir -p $(@D)
	$(CGC) -noentry -oglsl -profile vs_2_0 $< 
	$(XXD) -i $< $@.cpp
	$(CXX) -c $@.cpp -o $@
	rm $@.cpp

$(OBJDIR)%_f.o: %_f.glsl
	@mkdir -p $(@D)
	$(CGC) -noentry -oglsl -profile ps_2_0 $< 
	$(XXD) -i $< $@.cpp
	$(CXX) -c $@.cpp -o $@
	rm $@.cpp

$(OBJDIR)%.d: %.cpp
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
	$(LD) $(LDFLAGS) $(OBJFILES) $(LIBS) -o $(TARGETDIR)$(TARGET).html

clean:
	rm -rf $(OBJDIR) $(TARGETDIR)$(TARGET)$(TARGET_EXT)
	
cleandep:
	rm -f $(OBJFILES:.o=.d)

superclean:
	rm -rf $(OBJDIR) $(TARGETDIR)$(TARGET)$(TARGET_EXT) $(addsuffix /*~, $(MODULES)) *.elf *~

run: start_rule
	$(CURDIR)/$(TARGET)$(TARGET_EXT)

