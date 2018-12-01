LIBGL = -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lXxf86vm -lXcursor -lXinerama
LIBGLEW = -lGLEW
LIBGLAD = glad.c
INC = ./include
LIB = ./lib
LIBSOIL = -lSOIL


all : rabbit.elf


%.elf : %.cpp
	g++ $< $(LIBGLAD) $(LIBGL) $(LIBSOIL) -I $(INC) -L $(LIB) -o $@