LIBDIR:=lib
OUTPUT:=./bin
MQTT:=./$(LIBDIR)/mqtt
COMMON:=./$(LIBDIR)/common
MAIN:=../$(LIBDIR)/main
WKSTACK:=./$(LIBDIR)/WKStack
UDPSERVER:=./$(LIBDIR)/udpserver
PLATFORM:=./$(LIBDIR)/platform

P?=linux

CSRCS := $(wildcard *.c $(COMMON)/*.c $(MQTT)/*.c $(WKSTACK)/*.c $(UDPSERVER)/*.c $(PLATFORM)/$(P)/*.c)
OBJS = $(CSRCS:.c = .o)

CC = gcc

INCLUDES := \
	-I$(COMMON) \
	-I$(MQTT)  \
	-I$(PLATFORM)  \
	-I$(WKSTACK)  \
	-I$(UDPSERVER)  \
	-I$(PLATFORM)/${P}  

LIBS = -L/

CCFLAGS = -g -w -O0 -lpthread -lssl -lcrypt -lcrypto -lrt $(INCLUDES) -D_PLATFORM_=${P}


all: $(OBJS) ./main.c
	$(CC) $^ -o $(OUTPUT)/WKdevice $(CCFLAGS)


clean:
	rm -f $(MQTT)/*.o
	rm -f $(COMMON)/*.o
	rm -f $(MAIN)/*.o
	rm -f $(OUTPUT)/*

cl:
	rm -f ./log/*
