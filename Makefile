all: 
	gcc -o fileCompressor fileCompressor.c

clean:
	rm -f fileCompressor HuffmanCodebook
