CC :=g++
SOURCES :=$(shell find ./ -name "*.cpp")
EXECUTABLE :=$(SOURCES:.c=)
TARGET_EXEC :=

INC_FLAGS := $(addprefix -I,./)
CFLAGS ?= $(INC_FLAGS) -O3 -Wall -Wextra -MMD -MP -shared -fPIC

all: 
	$(CC) $(CFLAGS) $(SOURCES) -o libseekgate.so

install:
	cp ./libseekgate.so /usr/lib
