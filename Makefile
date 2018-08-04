CC = g++ -g

COMPILER_FLAGS = -w -std=c++11 -DRES_PACK='"default"'

OBJ_NAME = ant-wars-HD

OBJS = src/main.cpp 

LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_gfx -lSDL2_ttf

INSTALL_DIR = /usr/local/bin/

all: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

install: $(OBJ_NAME)
	cp $(OBJ_NAME) $(INSTALL_DIR)

debug: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -g -o $(OBJ_NAME)
