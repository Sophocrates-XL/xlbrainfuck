// The translator application which translates a block of brainfuck code into C code.
#include <stdio.h>
#include <stdlib.h>
#include "xlbrainfuck.h"



// Execution in command line: xlbftranslator memsize bfsrc cdest
int main(int argc, char **argv) {
	
	// Ensures that the correct number of arguments has been passed.
	if (argc != 4) {
		fprintf(stderr, "You must specify the size of memory allocated to the brainfuck environment, the brainfuck source file, and the C destination file.\n");
		fprintf(stderr, "Follow this format in command line: xlbftranslator memsize bfsrc cdest.\n");
		return 1;
	}

	// Processes the arguments and determines if they are valid.
	const long memsize = strtol(argv[1], nullptr, 10);
	if (memsize <= 0) {
		fprintf(stderr, "You must supply a valid positive integer for the size of memory allocated.\n");
		return 1;
	}
	const char *bfsrc = argv[2];
	FILE *bfsrcfp = fopen(bfsrc, "rb");
	if (bfsrcfp == nullptr) {
		fprintf(stderr, "Invalid brainfuck source file.\n");
	}
	const char *cdest = argv[3];
	FILE *cdestfp = fopen(cdest, "wb");
	if (cdestfp == nullptr) {
		fprintf(stderr, "Unable to create C destination file.\n");
	}

	xl_brainfuck_env<int> bfe(memsize);

	// Copies file content into a buffer.
	fprintf(stderr, "Reading brainfuck source file ...\n");
	fseek(bfsrcfp, 0, SEEK_END);
	size_t filesize = ftell(bfsrcfp);
	fprintf(stderr, "Source file size: %d.\n", filesize);
	rewind(bfsrcfp);
	char *bfcodebuffer = (char *)calloc(filesize + 1, sizeof(char));
	fread(bfcodebuffer, 1, filesize, bfsrcfp);
	fprintf(stderr, "Content read:\n%s\n", bfcodebuffer);

	// Translates brainfuck code into C code and stores the content.
	size_t ccodesize = filesize * 32;
	char *ccodebuffer = (char *)calloc(ccodesize + 1, sizeof(char));
	fprintf(stderr, "Translating brainfuck code to C code ...\n");
	bfe.translate(bfcodebuffer, ccodebuffer);
	fprintf(stderr, "Translated C code:\n%s\n", ccodebuffer);

	// Writes content to destination file.
	fprintf(stderr, "Writing into C destination file ...\n");
	fprintf(cdestfp, ccodebuffer);

	fprintf(stderr, "Operation complete.\n");
	// Frees up resources.
	free(bfcodebuffer);
	free(ccodebuffer);
	fclose(bfsrcfp);
	fclose(cdestfp);

	return 0;

}