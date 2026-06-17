#set environment variable RM_INCLUDE_DIR to the location of redismodule.h
ifndef RM_INCLUDE_DIR
	RM_INCLUDE_DIR=../
endif

ifndef RMUTIL_LIBDIR
	RMUTIL_LIBDIR=../rmutil
endif

# find the OS
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

# Build mode: default to a stripped/hardened release build.
DEBUG ?= 0
STRIP ?= strip

# Compile flags for linux / osx
ifeq ($(uname_S),Linux)
	SHOBJ_CFLAGS ?= -fno-common
	SHOBJ_LDFLAGS ?= -shared -Bsymbolic
	HARDEN_LDFLAGS ?= -Wl,--gc-sections -Wl,--as-needed -Wl,-z,relro -Wl,-z,now
	STRIP_LDFLAGS ?=
else
	SHOBJ_CFLAGS ?= -dynamic -fno-common
	SHOBJ_LDFLAGS ?= -bundle -undefined dynamic_lookup
	HARDEN_LDFLAGS ?= -Wl,-dead_strip
	STRIP_LDFLAGS ?= -Wl,-x
endif

COMMON_CFLAGS ?= -I$(RM_INCLUDE_DIR) -Wall -fPIC -std=gnu99
RELEASE_CFLAGS ?= -O2 -ffunction-sections -fdata-sections -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fvisibility=hidden
DEBUG_CFLAGS ?= -O0 -g -ggdb
LIBS ?= -lm

CFLAGS = $(COMMON_CFLAGS) $(SHOBJ_CFLAGS)
ifeq ($(DEBUG),1)
	CFLAGS += $(DEBUG_CFLAGS)
	MODULE_LDFLAGS = $(SHOBJ_LDFLAGS)
else
	CFLAGS += $(RELEASE_CFLAGS)
	MODULE_LDFLAGS = $(SHOBJ_LDFLAGS) $(HARDEN_LDFLAGS) $(STRIP_LDFLAGS)
endif

CC=gcc

all: rmutil module.so

rmutil: FORCE
	$(MAKE) -C $(RMUTIL_LIBDIR)

module.o: module.c cmd.h sha256.h

sha256.o: sha256.c sha256.h

cmd.o: cmd.c cmd.h

module.so: module.o sha256.o cmd.o
	$(CC) -o $@ module.o sha256.o cmd.o $(MODULE_LDFLAGS) -L$(RMUTIL_LIBDIR) -lrmutil $(LIBS)
ifeq ($(DEBUG),0)
	$(STRIP) --strip-unneeded $@
endif

clean:
	rm -rf *.xo *.so *.o

FORCE:
