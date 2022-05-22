all: send recv

# macOS style
# send: base64_utils encode decode base64_utils.h cencode.h cdecode.h
# 	gcc send.c base64_utils.o cencode.o cdecode.o -o send

# Other
send: base64_utils encode decode base64_utils.h cencode.h cdecode.h
	gcc send.c base64_utils.o cencode.o cdecode.o base64_utils.h cencode.h cdecode.h -o send

recv: recv.c
	gcc recv.c -o recv
base64_utils: base64_utils.c
	gcc -c base64_utils.c -o base64_utils.o
encode: cencode.c
	gcc -c cencode.c -o cencode.o
decode: cdecode.c
	gcc -c cdecode.c -o cdecode.o
clean:
	rm -rf *.o send recv