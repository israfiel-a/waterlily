CC=gcc 
AR=ar
CFLAGS=-Wall -Wextra -Wpedantic -Werror -IInclude -DFILENAME=\"$(notdir $<)\"
LDFLAGS=-nostartfiles
ARFLAGS=rcs

SRCFILES=Engine.c Files.c Input.c Public.c Vulkan.c Window.c
SRCS=$(foreach src, $(SRCFILES), $(addprefix Source/, $(src)))
OBJS=$(SRCS:.c=.o)
LIB=libWaterlily.a

DBGDIR=Debug
DBGLIB=$(DBGDIR)/$(LIB)
DBGOBJS=$(addprefix $(DBGDIR)/, $(OBJS))
DBGCFLAGS=-Og -g3 -ggdb -fanalyzer -fsanitize=leak -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=undefined
DBGLDFLAGS=-fsanitize=address -fsanitize=undefined

RELDIR=Release
RELLIB=$(RELDIR)/$(LIB)
RELOBJS=$(addprefix $(RELDIR)/, $(OBJS))
RELCFLAGS=-march=native -mtune=native -Ofast -flto
RELLDFLAGS=-Ofast -flto

.PHONY: all clean debug debug_prep release_prep release install

all: release

debug: debug_prep $(DBGLIB)

$(DBGLIB): $(DBGOBJS)
	$(AR) $(ARFLAGS) $(DBGLIB) $(DBGOBJS)

$(DBGDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(DBGCFLAGS) $(LDFLAGS) $(DBGLDFLAGS) -o $@ $<

release: release_prep $(RELLIB)

$(RELLIB): $(RELOBJS)
	$(AR) $(ARFLAGS) $(RELLIB) $(RELOBJS)

$(RELDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $(RELCFLAGS) $(LDFLAGS) $(RELLDFLAGS) -o $@ $<

release_prep:
	@mkdir -p $(RELDIR)/Source

debug_prep:
	@mkdir -p $(DBGDIR)/Source 

clean:
	rm -rf $(RELDIR) $(DBGDIR)

