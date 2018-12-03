LIBGL = -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lXxf86vm -lXcursor -lXinerama
LIBGLEW = -lGLEW -D USE_GLEW
LIBGLAD = glad.c
INC = ./include
LIB = ./lib
LIBSOIL = -lSOIL
HEADERS = Shader.hpp
USE_GLEW = 0


ifeq ($(USE_GLEW),1)
LIBGL += $(LIBGLEW)
else
LIBGL += $(LIBGLAD)
endif


all : bunny.elf

clean:
	rm -rf *.elf


%.elf : %.cpp $(HEADERS)
	g++ $< $(LIBGL) $(LIBSOIL) -I$(INC) -L$(LIB) -o $@