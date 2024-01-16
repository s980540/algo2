# Project: algo
# Makefile created by Steve Jhang
# Date modified: 2018.01.03

CPP			= g++.exe -mno-ms-bitfields
CC			= gcc.exe -mno-ms-bitfields
WINDRES		= windres.exe
SOURCE		= $(wildcard *.c)
OBJS		= $(patsubst %.c, %.o, $(SOURCE))
LINKOBJS	= $(patsubst %.c, %.o, $(SOURCE))
LIBS      	= -L"C:/MinGW/lib" \
			  -L"C:/MinGW/mingw32/lib" \
			  -static-libgcc -m32
INCS      	= -I"C:/MinGW/include" \
			  -I"C:/MinGW/mingw32/include" \
			  -I"C:/MinGW/lib/gcc/mingw32/5.3.0/include"
CXXINCS   	= -I"C:/MinGW/include" \
			  -I"C:/MinGW/mingw32/include" \
			  -I"C:/MinGW/lib/gcc/mingw32/5.3.0/include" \
			  -I"C:/MinGW/lib/gcc/mingw32/5.3.0/include/c++"

BIN      = algo2.exe
CXXFLAGS = $(CXXINCS) -m32
CFLAGS   = $(INCS) -g -m32
RM       = rm.exe -f
BFLAGS   = -static -lpthread -g -o

.PHONY: all clean

# all: clean $(BIN)
all: $(BIN)

clean:
	${RM} $(OBJS)

$(BIN): $(OBJS)
	$(CC) $(LINKOBJS) $(BFLAGS) $(BIN) $(LIBS)

main.o: main.c
	$(CC) -c main.c $(BFLAGS) main.o $(CFLAGS)