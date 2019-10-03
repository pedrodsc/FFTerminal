build:
	$ gcc -o ffterminal main.c fft.c -lncurses -lm -lasound
run:
	$ ./ffterminal deafult
all:
	$ gcc -o ffterminal main.c fft.c -lncurses -lm -lasound
	$ ./ffterminal deafult
