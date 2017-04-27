MQTT:=./mqtt
COMMON:=./common
OUTPUT:=./bin
MAIN:=../main
WKSTACK:=./WKStack
PLATFORM:=./platform

P?=linux


CSRCS := $(wildcard *.c $(COMMON)/*.c $(MQTT)/*.c $(WKSTACK)/*.c $(PLATFORM)/$(P)/*.c)
OBJS = $(CSRCS:.c = .o)

CC = gcc

INCLUDES := \
	-I$(COMMON) \
	-I$(MQTT)  \
	-I$(PLATFORM)  \
	-I$(WKSTACK)  \
	-I$(PLATFORM)/${P}  \

LIBS = -L/

CCFLAGS = -g -w -O0 -lpthread -lssl -lcrypt -lcrypto -lrt $(INCLUDES) -D_PLATFORM_=${P}


all: $(OBJS) ./main/device.c
	$(CC) $^ -o $(OUTPUT)/WKdevice $(CCFLAGS)


clean:
	rm -f $(MQTT)/*.o
	rm -f $(COMMON)/*.o
	rm -f $(MAIN)/*.o
	rm -f $(OUTPUT)/linux/WKdevice

cl:
	rm -f ./log/*
