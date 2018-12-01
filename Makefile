LIBGL = -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lXxf86vm -lXcursor -lXinerama
LIBGLEW = -lGLEW -D USE_GLEW
LIBGLAD = glad.c
INC = ./include
LIB = ./lib
LIBSOIL = -lSOIL
USE_GLEW = 1


ifeq ($(USE_GLEW),1)
LIBGL += $(LIBGLEW)
else
LIBGL += $(LIBGLAD)
endif


all : rabbit.elf


%.elf : %.cpp
	g++ $< $(LIBGL) $(LIBSOIL) -I$(INC) -L$(LIB) -o $@