CC=g++
CFLAGS=-O0 -g -std=c++17 -Wall -Wextra -Wshadow -pthread#-DNDEBUG
LIBS=
LPATHS=
INCLUDES=alloc.h
# define the C source files
SRCS=main.cpp alloc.cpp
# suffix replacement $(name:string1=string2)
OBJS=$(SRCS:.cpp=.o)
MAIN=main

.PHONY: clean

all : $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LPATHS) $(LIBS)

# Create object files of source code
%.o: %.cpp $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<

# Clean object files
clean:
	$(RM) *.o
	$(RM) $(MAIN)
