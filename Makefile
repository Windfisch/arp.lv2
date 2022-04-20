arp.so: arp.cpp
	g++ -std=c++20 -fPIC -shared arp.cpp -o arp.so
