CC = g++

COMPILER_FLAGS = -w

OBJ_NAME = "ant wars HD"

OBJS = src/main.cpp 

LINKER_FLAGS = -lSDL2 -lSDL2_image

all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
