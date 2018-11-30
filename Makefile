LIBGL = -lGLEW -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl
INC = ./include
LIB = ./lib
LIBSOIL = -lSOIL


all : rabbit.elf

%.elf : %.cpp
	g++ $< $(LIBGL) $(LIBSOIL) -I $(INC) -L $(LIB) -o $@