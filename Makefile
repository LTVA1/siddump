CC=gcc
CXX=g++
CFLAGS+=-O3 -Wall
CXXFLAGS=$(CFLAGS)

siddump.exe: siddump.o
	gcc -o $@ $^ -lm
	strip $@
	
	@rm -rf siddump.o
