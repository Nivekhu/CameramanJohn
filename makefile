CC = g++
CFLAGS = -02 -g -c
OpenCV = `pkg-config --cflags --libs opencv`

.C.o:
	$(CC) $(CFLAGS) $< -o $@
all:
	$(CC) main.c -o main $(OpenCV)
