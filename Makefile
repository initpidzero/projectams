#!/bin/sh
gcc -c library.c -lcrypto -DDEBUG;
gcc input.c library.o -o input -DDEBUG -lcrypto;
gcc output.c library.o -o output -DDEBUG -lcrypto;
gcc -c udp_library.c -lcrypto -DDEBUG;
gcc udp_client.c library.o udp_library.o -o udp_client -DDEBUG -lcrypto;
gcc udp_server.c library.o udp_library.o -o udp_server -DDEBUG -lcrypto;
