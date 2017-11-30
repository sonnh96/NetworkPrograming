# the compiler: gcc for C program, define as g++ for C++
CC = gcc -pthread

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -Wall

# the list of header files
#INCL   = server.h
#the list of source files
SRC = client.c handle.c

# the build target executable:
TARGET = client


$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

$(OBJ): $(INCL)

clean:
	$(RM) $(TARGET)
