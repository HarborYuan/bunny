LIBGL = -lGLEW -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lXxf86vm -lXcursor -lXinerama
INC = ./include
LIB = ./lib
LIBSOIL = -lSOIL


all : rabbit.elf

%.elf : %.cpp
	g++ $< $(LIBGL) $(LIBSOIL) -I $(INC) -L $(LIB) -o $@