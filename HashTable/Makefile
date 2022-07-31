CC=g++
CFLAGS=-O0 -g -std=c++17 -Wall -Wextra -Wshadow -pthread -lrt
# CFLAGS=-DNDEBUG -O3 -std=c++17 -Wall -Wextra -Wshadow -pthread -lrt
LIBS=
LPATHS=
INCLUDES=table.h utils.h
# define the C source files
SRCS=table.cpp utils.cpp
# suffix replacement $(name:string1=string2)
OBJS=$(SRCS:.cpp=.o)
SERVER=server
CLIENT=client

.PHONY: clean

all : $(SERVER) $(CLIENT)

$(SERVER): $(OBJS) server.o
	$(CC) $ -o $@ $^ $(LPATHS) $(LIBS) $(CFLAGS)

$(CLIENT): $(OBJS) client.o
	$(CC) -o $@ $^ $(LPATHS) $(LIBS) $(CFLAGS)

# Create object files of source code
%.o: %.cpp $(INCLUDES)
	$(CC) -o $@ -c $< $(CFLAGS)

# Clean object files
clean:
	$(RM) *.o
	$(RM) $(SERVER)
	$(RM) $(CLIENT)
