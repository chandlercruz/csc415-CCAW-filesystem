#**************************************************************
# Class: CSC-415
# Name: Team CCAW - Aaron Colmenares, Chandler Cruz, Wesley Xu, Chaoyi Ying
# Student ID: 916913613 917048657 916260714 918810235
# Project: Basic File System - PentaFS
#
# File: Makefile
#
# Description: contain comands that run Penta File System
#
#**************************************************************/


VOLUMENAME=SampleVolume
VOLUMESIZE=10000000
BLOCKSIZE=512
CC=gcc
CFLAGS= -g -lm -lreadline -I.
LIBS =pthread
DEPS = 
ADDOBJ= fsLow.o b_io.o bitMap.o mfs.o fsMakeVol.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

all:
	make fsshell
	make fsFormat
	make fileExplorer

fsshell: fsshell.o $(ADDOBJ)
	$(CC) -o $@ $^ $(CFLAGS) -l $(LIBS)

fsFormat: fsFormat.o $(ADDOBJ)
	$(CC) -o $@ $^ $(CFLAGS) -l $(LIBS)

fileExplorer: fileExplorer.o $(ADDOBJ)
	$(CC) -o $@ $^ $(CFLAGS) -l $(LIBS)

clean:
	rm *.o fsshell fileExplorer fsFormat

explore:
	make fileExplorer
	./fileExplorer $(VOLUMENAME)

format:
	make fsFormat
	./fsFormat $(VOLUMENAME) $(VOLUMESIZE) $(BLOCKSIZE)

run:
	make fsshell
	./fsshell $(VOLUMENAME)

wipe:
	rm $(VOLUMENAME)
