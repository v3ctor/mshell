INC=-Iinclude
override CFLAGS += $(INC) -Wall -Wextra -Werror

PARSERDIR=input_parse

SRCS=src/utils.c src/mshell.c src/builtins.c src/io.c src/errors.c src/pipeline.c src/sys.c src/signals.c
OBJS:=$(SRCS:src/%.c=build/%.o)

all: mshell

debug:
	make all CFLAGS="-DDEBUG -g2 -fsanitize=address"

mshell: $(OBJS) siparse.a
	cc $(CFLAGS) $(OBJS) build/siparse.a -o build/$@ 

build/%.o: src/%.c
	@mkdir -p build/
	cc $(CFLAGS) -c $< -o $@

siparse.a:
	$(MAKE) -C $(PARSERDIR)

clean:
	rm -f build/mshell build/*.o
