all: arp.so plugin
plugin:
	mkdir -p arp.lv2
	cp arp.so arp.ttl manifest.ttl arp.lv2/

arp.so: arp.cpp
	g++ -std=c++20 -fPIC -shared arp.cpp -o arp.so

clean:
	rm arp.so

compile_commands.json: Makefile
	bear -- make clean all
