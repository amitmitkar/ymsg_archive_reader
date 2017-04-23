SRCS:=ymsg_parse.c ymsg_hdr.h

ymsg_parse:$(SRCS)
	gcc -g -o $@ $^

all:ymsg_parse

clean:
	rm -f ymsg_parse

