// VALIDATED. PERFORM MORE TESTING ON SAMPLE PROGRAMS.

#include <stdio.h>
#include <string.h>
#include "conio.h"
#include "xlbrainfuck.h"



constexpr size_t INTERPRETER_BUFFER_SIZE = 1024;
constexpr size_t CODE_BUFFER_SIZE = 1024;
constexpr size_t LINE_BUFFER_SIZE = 1024;
constexpr size_t VIEW_START_INDEX = 0;
constexpr size_t VIEW_END_INDEX = 15;



int main() {

	char codebuffer[CODE_BUFFER_SIZE + 1] = { 0 };
	char linebuffer[LINE_BUFFER_SIZE + 1] = { 0 };
	char isdebug = false;

	xl_brainfuck_env<int> bfe(INTERPRETER_BUFFER_SIZE);

	printf("== XL BRAINFUCK CONSOLE ==\n");
	printf("# Enter reset to reinitialize the brainfuck environment.\n");
	printf("# Other inputs will be interpreted as brainfuck code.\n");

	while (true) {

		char *codeptr = codebuffer;

		// Accepts multiple lines of code until the buffer is full or an empty row is encountered.
		while (true) {
			printf("COMMAND ");
			fgets(linebuffer, LINE_BUFFER_SIZE, stdin);
			if (strcmp(linebuffer, "reset\n") == 0) {
				bfe.reset();
				printf("CONSOLE: Environment reset.\n");
				break;
			}
			strncpy(codeptr, linebuffer, codebuffer + CODE_BUFFER_SIZE - codeptr);
			while (*codeptr != 0) codeptr++;
			if (strcmp(linebuffer, "\n") == 0 || codeptr >= codebuffer + CODE_BUFFER_SIZE) {
				break;
			}
			// Resets the line buffer.
			memset(linebuffer, 0, sizeof(char) * (LINE_BUFFER_SIZE + 1));
		}

		// Interprets the code.
		printf("OUTPUT: ");
		bfe.interpret(codebuffer);
		printf("\n");

		// Resets the code buffer and its ptr.
		memset(codebuffer, 0, sizeof(char) * (CODE_BUFFER_SIZE + 1));
		codeptr = codebuffer;

	}

	return 0;

}