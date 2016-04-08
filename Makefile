all: mini3d.c linux.h 
	gcc mini3d.c -o mini3d -g -lxcb -lm -lxcb-image -lxcb-util -I./
