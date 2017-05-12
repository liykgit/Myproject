LIBDIR:=lib
OBJDIR:=obj
OUTPUT:=./output
MQTT:=$(LIBDIR)/mqtt
COMMON:=$(LIBDIR)/common
WKSTACK:=$(LIBDIR)/WKStack
UDPSERVER:=$(LIBDIR)/udpserver
PLATFORM:=$(LIBDIR)/platform

P?=linux

CSRCS := $(wildcard $(COMMON)/*.c $(MQTT)/*.c $(WKSTACK)/*.c $(UDPSERVER)/*.c $(PLATFORM)/$(P)/*.c)

CC = gcc

INCLUDES := \
	-I$(COMMON) \
	-I$(MQTT)  \
	-I$(PLATFORM)  \
	-I$(WKSTACK)  \
	-I$(UDPSERVER)  \
	-I$(PLATFORM)/${P}  


CCFLAGS = -g -w -O0 -lpthread -lssl -lcrypt -lcrypto -lrt $(INCLUDES)

VENGA_BIN=device

VENGA_LIB=libvenga.a


default: all

objs: 
	$(CC) -c $(CSRCS) $(CCFLAGS) 

lib: objs
	$(AR) rc $(OUTPUT)/$(VENGA_LIB) $(wildcard ./*.o)

all: lib
	$(CC) main.c $(CSRCS) $(CCFLAGS) -o $(OUTPUT)/$(VENGA_BIN)


clean:
	rm -f ./*.o
	rm -f $(MQTT)/*.o
	rm -f $(COMMON)/*.o
	rm -f $(OUTPUT)/*

cl:
	rm -f ./log/*
