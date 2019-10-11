build:
	$ gcc -o ffterminal main.c fft.c -lncurses -lm -lasound -Os -ffast-math
run:
	$ ./ffterminal
all:
	$ gcc -o ffterminal main.c fft.c -lncurses -lm -lasound -O5 -ffast-math
	$ ./ffterminal
debug:
	$ gcc -g -Wall -o ffterminal main.c fft.c -lncurses -lm -lasound
	$ gcc -g -Wall -o a teste.c fft.c -lm -lncurses
