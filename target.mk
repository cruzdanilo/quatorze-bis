# http://make.mad-scientist.net/papers/multi-architecture-builds/
.SUFFIXES:

ifndef _ARCH
_ARCH := $(shell python -c "from sys import platform; print platform")
export _ARCH
endif

OBJDIR := _$(_ARCH)

MAKETARGET = $(MAKE) --no-print-directory -C $@ -f $(CURDIR)/Makefile \
		SRCDIR=$(CURDIR) $(MAKECMDGOALS)

.PHONY: $(OBJDIR)
$(OBJDIR):
	+@[ -d $@ ] || mkdir -p $@
	+@$(MAKETARGET)

Makefile : ;
%.mk :: ;

% :: $(OBJDIR) ;

.PHONY: clean
clean:
	rm -rf $(OBJDIR)
