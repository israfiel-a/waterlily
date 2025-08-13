BUILD=build
SOURCE=Source
INCLUDE=Include
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

CC=gcc 
AR=ar
CFLAGS=-Wall -Wextra -Wpedantic -Werror -I$(INCLUDE) 
LDFLAGS=-nostartfiles
ARFLAGS=rcs

SRCFILES=Engine Files Input Public Vulkan Window
SRCS=$(foreach src, $(SRCFILES), $(addprefix $(SOURCE)/, $(src)).c)
OBJS=$(foreach obj, $(SRCFILES), $(addprefix $(BUILD)/, $(obj)).o)
LIB=$(BUILD)/libWaterlily.a

DBGOBJS=$(addprefix $(DBGDIR)/, $(OBJS))
DBGCFLAGS=-Og -g3 -ggdb -fanalyzer -fsanitize=leak -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined
DBGLDFLAGS=-fsanitize=address -fsanitize=undefined

RELOBJS=$(addprefix $(RELDIR)/, $(OBJS))
RELCFLAGS=-march=native -mtune=native -Ofast -flto
RELLDFLAGS=-Ofast -flto

.PHONY: all clean prep install

all: prep $(LIB) 

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $(LIB) $(OBJS)

$(BUILD)/%.o: $(SOURCE)/%.c
ifdef debug
	$(CC) -c $(CFLAGS) -DFILENAME=\"$(notdir $<)\" $(DBGCFLAGS) $(LDFLAGS) $(DBGLDFLAGS) -o $@ $<
else
	$(CC) -c $(CFLAGS) -DFILENAME=\"$(notdir $<)\" $(RELCFLAGS) $(LDFLAGS) $(RELLDFLAGS) -o $@ $<
endif

install: prep $(LIB)
	install -d $(DESTDIR)$(PREFIX)/lib/ 
	install -d $(DESTDIR)$(PREFIX)/include 
	install -m 644 $(LIB) $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(LIB) $(DESTDIR)$(PREFIX)/include/

prep:
	@mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

