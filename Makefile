BUILD=$(abspath build)
SOURCE=$(abspath Source)
INCLUDE=$(abspath Include)
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

CFLAGS=-std=gnu2x -Wall -Wextra -Wpedantic -Werror -I$(INCLUDE) 
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

export_commands: prep
ifeq (, $(shell which jq))
	$(error "The JQ JSON utility was not found.")
endif
	$(MAKE) --always-make --dry-run | grep -wE 'cc|gcc|clang' | grep -w '\-c' | jq -nR '[inputs|{directory:"$(BUILD)", command:., file:match(" [^ ]+$$").string[1:], output:"$(BUILD)"+match(" [^ ]+$$").string[1+("$(SOURCE)"|length):-2]+".o"}]' > $(BUILD)/compile_commands.json

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $(LIB) $(OBJS)

$(BUILD)/%.o: $(SOURCE)/%.c
ifdef debug
	$(CC) -c $(CFLAGS) -DFILENAME=\"$(notdir $<)\" $(DBGCFLAGS) $(LDFLAGS) $(DBGLDFLAGS) -o $@ $<
else
	$(CC) -c $(CFLAGS) -DFILENAME=\"$(notdir $<)\" $(RELCFLAGS) $(LDFLAGS) $(RELLDFLAGS) -o $@ $<
endif

install: prep $(LIB)
	install -d $(DESTDIR)$(PREFIX)/lib 
	install -d $(DESTDIR)$(PREFIX)/include 
	install -m 644 $(LIB) $(DESTDIR)$(PREFIX)/lib
	install -m 644 $(INCLUDE)/Waterlily.h $(DESTDIR)$(PREFIX)/include

prep:
	@mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

