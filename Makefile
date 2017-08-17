# http://make.mad-scientist.net/papers/multi-architecture-builds/
ifeq (,$(filter _%,$(notdir $(CURDIR))))
include target.mk
else
#----- End Boilerplate

TARGET_NAME ?= 14-bis

3PDIR := $(SRCDIR)/third_party
RETRO := $(3PDIR)/libretro-common
V8 := $(3PDIR)/v8

VPATH := $(SRCDIR)/src $(RETRO)
SRCS := 14-bis.cc base/global.cc base/console.cc encodings/encoding_utf.c \
		compat/compat_strl.c glsym/rglgen.c glsym/glsym_gl.c
CPPFLAGS += -I$(SRCDIR)/src -I$(RETRO)/include -I$(V8)/include
CCCFLAGS ?= -Wall
TARGET := $(TARGET_NAME).so
LDFLAGS += -shared -Wl,--version-script=$(SRCDIR)/src/14-bis.map \
		-Wl,--no-undefined
LDLIBS += -Wl,--start-group $(V8_LIBS) -Wl,--end-group -lrt -ldl -lpthread \
		-lc++ -lGL
CCCFLAGS += -fPIC
LDFLAGS += -fPIC

ifeq ($(DEBUG),1)
CCCFLAGS += -O0 -g
else
CCCFLAGS += -O3
endif
CFLAGS += -std=c99 $(CCCFLAGS)
CXXFLAGS += -std=c++11 $(CCCFLAGS)
OBJS := $(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(SRCS)))
OBJDIRS := $(addprefix $(CURDIR)/,$(sort $(dir $(OBJS))))

V8_GNOUT := v8
V8_GNARGS := is_debug=false target_cpu="x64" is_component_build=false \
		v8_static_library=true
V8_LIBDIR := $(V8_GNOUT)/obj
V8_LIBS := $(addprefix $(V8_LIBDIR)/,libv8_base.a libv8_libbase.a \
		libv8_external_snapshot.a libv8_libplatform.a libv8_libsampler.a \
		third_party/icu/libicuuc.a third_party/icu/libicui18n.a \
		src/inspector/libinspector.a)
V8_TOOLS := $(3PDIR)/depot_tools
export PATH := $(V8_TOOLS):$(PATH)

.PHONY: all run update v8
all: $(TARGET)

run: all
	retroarch -v -L $(TARGET) $(SRCDIR)/samples/main.js

update: | $(3PDIR)
	git pull
	git submodule update --init --remote
	cd $(3PDIR) && gclient sync || fetch v8

v8: | $(V8)
	cd $(V8) && gn gen $(CURDIR)/$(V8_GNOUT) --args='$(V8_GNARGS)'
	ninja v8 -C $(V8_GNOUT)

$(3PDIR) $(OBJDIRS):
	mkdir -p $@

$(V8_TOOLS):
	git submodule update --init --remote

$(RETRO):
	git submodule update --init --remote

$(V8): | $(V8_TOOLS) $(3PDIR)
	cd $(3PDIR) && gclient sync || fetch v8

%.a: v8;

$(OBJS): | $(OBJDIRS)

$(TARGET): $(V8_LIBS) $(OBJS) | $(RETRO) $(V8)
	$(LINK.cc) $(OBJS) $(LOADLIBES) $(LDLIBS) -o $@
	cp $(V8_GNOUT)/*.bin .
	cp $(V8_GNOUT)/icudtl.dat .

# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
DEPFLAGS = -MT $@ -MMD -MP -MF $*.Td
COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
POSTCOMPILE = @mv -f $*.Td $*.d && touch $@

%.o : %.c
%.o : %.c %.d
	$(COMPILE.c) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

%.o : %.cc
%.o : %.cc %.d
	$(COMPILE.cc) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

%.d: ;
.PRECIOUS: %.d

include $(wildcard $(addsuffix .d,$(basename $(SRCS))))

#----- Begin Boilerplate
endif
