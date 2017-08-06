TARGET_NAME ?= quatorze-bis

OBJECTS := quatorze-bis.o encoding_utf.o compat_strl.o
3PDIR := third_party
RETRO := $(3PDIR)/libretro-common
V8 := $(3PDIR)/v8
V8_GNOUT := out.gn/$(TARGET_NAME)
V8_GNARGS := is_debug=false target_cpu="x64" is_component_build=false v8_static_library=true
V8_LIBDIR := $(V8)/$(V8_GNOUT)/obj
V8_LIBS := $(addprefix $(V8_LIBDIR)/,libv8_base.a libv8_libbase.a \
libv8_external_snapshot.a libv8_libplatform.a libv8_libsampler.a \
third_party/icu/libicuuc.a third_party/icu/libicui18n.a src/inspector/libinspector.a)
V8_TOOLS := $(3PDIR)/depot_tools
export PATH := $(CURDIR)/$(V8_TOOLS):$(PATH)

TARGET := $(TARGET_NAME).so
VPATH := src $(addprefix $(RETRO)/,encodings compat)
CPPFLAGS += -I$(RETRO)/include -I$(V8)/include
CCCFLAGS ?= -Wall
LDFLAGS += -shared -Wl,--no-undefined -Wl,--version-script=src/quatorze-bis.map
LDLIBS += -Wl,--start-group $(V8_LIBS) -Wl,--end-group -lrt -ldl -lpthread -lc++
CCCFLAGS += -fPIC
LDFLAGS += -fPIC

ifeq ($(DEBUG),1)
CCCFLAGS += -O0 -g
else
CCCFLAGS += -O3
endif
CFLAGS += -std=c99 $(CCCFLAGS)
CXXFLAGS += -std=c++11 $(CCCFLAGS)

.PHONY: all clean run update v8 v8-clean
all: $(TARGET)

clean:
	rm -rf $(OBJECTS) $(TARGET) $(DEPDIR) *.bin icudtl.dat

run: all
	retroarch -v -L $(TARGET)

update: | $(3PDIR)
	git pull
	git submodule update --init --remote
	cd $(3PDIR) && gclient sync || fetch v8

v8: | $(V8)
	cd $(V8) && gn gen $(V8_GNOUT) --args='$(V8_GNARGS)' && ninja v8 -C $(V8_GNOUT)

v8-clean:
	rm -rf $(V8)/$(V8_GNOUT)

$(3PDIR):
	mkdir -p $@

$(V8_TOOLS):
	git submodule update --init --remote

$(RETRO):
	git submodule update --init --remote

$(V8): | $(V8_TOOLS) $(3PDIR)
	cd $(3PDIR) && gclient sync || fetch v8

%.a: v8;

$(TARGET): $(V8_LIBS) $(OBJECTS) | $(RETRO)
	$(LINK.cc) $(OBJECTS) $(LOADLIBES) $(LDLIBS) -o $@
	cp $(V8)/$(V8_GNOUT)/*.bin .
	cp $(V8)/$(V8_GNOUT)/icudtl.dat .

# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

%.o : %.c
%.o : %.c $(DEPDIR)/%.d
	$(COMPILE.c) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

%.o : %.cc
%.o : %.cc $(DEPDIR)/%.d
	$(COMPILE.cc) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

%.o : %.cxx
%.o : %.cxx $(DEPDIR)/%.d
	$(COMPILE.cc) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))
