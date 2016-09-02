pre:
	test -d bin || mkdir -p bin
main: 
	gcc -g main.c -o ./bin/main
map:
	gcc -g map.c -o ./bin/map
wc: pre
	gcc -g wc.c -o ./bin/wc
all: main map wc
clean:
	rm -r bin

