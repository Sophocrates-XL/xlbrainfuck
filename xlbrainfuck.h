/*
BRAINFUCK ENVIRONMENT:

< Preface >
# This brainfuck environment is written mainly for intellectual curiosity and for a deeper understanding of the principles of a Turing machine which the brainfuck language essentially mimics.

< Outline >
# Instantiation of the xl_brainfuck_env class establishes a brainfuck environment with a conceptual tape with a predefined number of storage units and the type of storage, as well as a pointer that initially points to the start of the tape. To calculate large numbers, the class is implemented as a template where the user may specify any type of storage unit that passes the std::is_integral<storage_t> test.
# The interpret method receives and interprets a block of code, which may or may not involve printing of the results onto the standard output. In the process, the internal states of the brainfuck environment such as the values on the tape and the location of pointer will be changed. To reinitialize the environment, the user must explicitly call the reset method, or subsequent calls of the interpret method will continue with the latest internal state.
# The translate method receives a block of brainfuck code from the source buffer, translates it into the corresponding C code, and stores it in the target buffer.

< Implementations >
# Code interpretation: interprets all the eight characters that define the brainfuck language. As per the language's specification, miscellaneous characters are simply ignored. Nonetheless, for more direct visualization of value, an additional ':' character is implemented to print the numerical value of the memory cell currently pointed to. For example, using "++++:" directly prints the result 4 whereas "++++." will print the character corresponding to ASCII code 4, which cannot be visualized.
# Translation of brainfuck code into C code: allows the user to translate a string of brainfuck code into a string of C-code. In this case, the ':' character available in interpretation is not included because it is not part of the brainfuck language standard. Some optimizations are done to collate several increments and decrements as well as pointer movements into single statements.

< Exceptions >
# Although no formal implementation for exceptions has been specified in the language standard, the following two error types will be thrown:
	$ Syntax error: occurs when the counts of '[' and ']' do not match, indicating an unenclosed loop. This is the only syntax error possible for brainfuck as all other characters operate on themselves and miscellaneous characters are ignored.
	$ Access violation: occurs when the pointer inside the environment attempts to read from or write into an address outside the environment's address boundaries.

< Application >
# The implementations in this header may interface with customized console or other applications.
# For demonstration, a console program has been written which receives and interprets multiple lines from standard input, and displays any results into the standard output. In addition, a translator program has been written which will create translate a file of brainfuck source code into a c source file.

< Comments >
# The header and the program rely on the <conio.h> for console input and output, which may not be supported by some systems.
# For translation, the default storage type of the brainfuck environment is assumed to be int.
*/

#pragma once

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <type_traits>



// Moves a pointer to the end of the current str.
// Used to concatenate multiple strings into the buffer.
void gotoend(char **ptr) {
	while (**ptr != 0) (*ptr)++;
}



// The xl_brainfuck_env class, where the template is provided to support multiple storage types.
template<typename storage_t>
class xl_brainfuck_env {

	// The tape and ptr variables implements the conceptualization of brainfuck as operating on a tape with a head pointing to one cell on the tape. In addition, the variables minaddr and maxaddr are defined to ensure that the pointer does not read beyond the memory assigned to the program.
	storage_t *tape;
	storage_t *ptr;
	storage_t *minaddr;
	storage_t *maxaddr;

public:

	// The constructor which fixes the storage size of the brainfuck environment upon instantiation.
	xl_brainfuck_env(size_t tapesize) {
		// Rejects non-integral storage types at compile time.
		static_assert(std::is_integral<storage_t>::value,
			"Cells in the tape must store integral values.");
		// Allocates an zero-initialized memory block with the program pointer pointing at the start of the block.
		this->tape = (storage_t *)calloc(tapesize, sizeof(storage_t));
		this->ptr = this->tape;
		this->minaddr = this->tape;
		this->maxaddr = this->tape + tapesize - 1;
	}
	// The destructor which frees the memory occupied by the tape.
	~xl_brainfuck_env() {
		free(tape);
	}

	// Interprets a block of brainfuck code which includes processing of memory units on the tape, reception of input and printing of output.
	// Only valid brainfuck command characters will be interpreted, while all other characters will be ignored except the '\0' at the end of the code string.
	// Validity of the pointer's address will be checked, where the interpreter will terminate and print an error if the ptr attempting to read or write a value is beyond the space defined by minaddr and maxaddr.
	int interpret(const char *code) {

		const char *codeptr = code;

		while (*codeptr != 0) {

			switch (*codeptr) {

			case '>': {
				this->ptr++;
				codeptr++;
				break;
			}

			case '<': {
				this->ptr--;
				codeptr++;
				break;
			}

			case '+': {
				if (this->ptroutofrange()) {
					printf("Access violation: attempt to write to an out-of-range address.");
					return 1;
				}
				(*this->ptr)++;
				codeptr++;
				break;
			}

			case '-': {
				if (this->ptroutofrange()) {
					printf("Access violation: attempt to write to an out-of-range address.");
					return 1;
				}
				(*this->ptr)--;
				codeptr++;
				break;
			}

			case '.': {
				if (this->ptroutofrange()) {
					printf("Access violation: attempt to read from an out-of-range address.");
					return 1;
				}
				printf("%c", *this->ptr);
				codeptr++;
				break;
			}

			// An additional character that prints out the numerical value instead of the character that the ptr points to.
			case ':': {
				if (this->ptroutofrange()) {
					printf("Access violation: attempt to read from an out-of-range address.");
					return 1;
				}
				printf("%d", *(this->ptr));
				codeptr++;
				break;
			}

			case ',': {
				if (this->ptroutofrange()) {
					printf("Access violation: attempt to write to an out-of-range address.");
					return 1;
				}
				*(this->ptr) = _getch();
				codeptr++;
				break;
			}

			case '[': {
				if (this->ptroutofrange()) {
					printf("Access violation: attempt to read from an out-of-range address.");
					return 1;
				}
				unsigned unsolvedloopcount = 1;
				const char *codesearchptr = codeptr + 1;
				bool hasloopmatch = false;
				bool hasloopskip = false;
				while (*codesearchptr != 0) {
					if (*codesearchptr == '[') {
						unsolvedloopcount++;
					} else if (*codesearchptr == ']') {
						unsolvedloopcount--;
						if (unsolvedloopcount == 0) {
							hasloopmatch = true;
							hasloopskip = *this->ptr == 0;
							break;
						}
					}
					codesearchptr++;
				}
				if (!hasloopmatch) {
					printf("Syntax error: unenclosed loop detected. Missing ']'.");
					return 1;
				}
				if (hasloopskip) {
					codeptr = codesearchptr + 1;
				} else {
					codeptr++;
				}
				break;
			}

			case ']': {
				unsigned unsolvedloopcount = 1;
				const char *codesearchptr = codeptr - 1;
				bool hasloopmatch = false;
				while (codesearchptr >= code) {
					if (*codesearchptr == ']') {
						unsolvedloopcount++;
					} else if (*codesearchptr == '[') {
						unsolvedloopcount--;
						if (unsolvedloopcount == 0) {
							hasloopmatch = true;
							codeptr = codesearchptr;
							break;
						}
					}
					codesearchptr--;
				}
				if (!hasloopmatch) {
					printf("Syntax error: unenclosed loop detected. Missing '['.");
					return 1;
				}
				break;
			}
			
			default: {
				codeptr++;
			}

			}

		}

		return 0;

	}

	// Translates a block of brainfuck code into the C source code according to the specifications of the instantiated environment.
	// No syntax checking or boundary checking are performed. Nonetheless, the function returns 0 if the code does not have unenclosed loops, 1 if the code has unenclosed loops.
	int translate(const char *bfcode, char *ccode) {

		// Determines string representing storage_t at compile time.
		// The Boolean type is not supported and will be substituted with char type.
		const char *DECLTYPE_STR = "";
		if (std::is_same<storage_t, char>::value) {
			DECLTYPE_STR = "char";
		} else if (std::is_same<storage_t, unsigned char>::value) {
			DECLTYPE_STR = "unsigned char";
		} else if (std::is_same<storage_t, short>::value) {
			DECLTYPE_STR = "short";
		} else if (std::is_same<storage_t, unsigned short>::value) {
			DECLTYPE_STR = "unsigned short";
		} else if (std::is_same<storage_t, int>::value) {
			DECLTYPE_STR = "int";
		} else if (std::is_same<storage_t, unsigned>::value) {
			DECLTYPE_STR = "unsigned";
		} else if (std::is_same<storage_t, long>::value) {
			DECLTYPE_STR = "long";
		} else if (std::is_same<storage_t, unsigned long>::value) {
			DECLTYPE_STR = "unsigned long";
		} else if (std::is_same<storage_t, long long>::value) {
			DECLTYPE_STR = "long long";
		} else if (std::is_same<storage_t, unsigned long long>::value) {
			DECLTYPE_STR = "unsigned long long";
		} else {
			DECLTYPE_STR = "char";
		}

		// Prepares pointers to direct translation.
		const char *bfptr = bfcode;
		char *cptr = ccode;

		
		// Prints code to the ccode str. Consists of pairs of sprintf and gotoend functions to print to the ccode str and relocate the pointer to the end of the new string.
		// Prints header inclusions.
		// The code depends on the conio.h header which is not part of the ANSI C standard.
		sprintf(cptr, "#include <stdio.h>\n");
		gotoend(&cptr);
		sprintf(cptr, "#include <stdlib.h>\n");
		gotoend(&cptr);
		sprintf(cptr, "#include <stddef.h>\n");
		gotoend(&cptr);
		sprintf(cptr, "#include \"conio.h\"\n");
		gotoend(&cptr);
		sprintf(cptr, "\n");
		gotoend(&cptr);
		
		// Prints the preparative codes of the program according to the configurations of the xl_brainfuck_env instance.
		// Stores the current level of indentation, which shall be incremented or decremented when a nested block is entered or exited.
		int indentlevel = 0;

		for (int i = 0; i < indentlevel; i++) {
			sprintf(cptr, "\t");
			gotoend(&cptr);
		}
		sprintf(cptr, "int main() {\n");
		gotoend(&cptr);
		indentlevel++;
		for (int i = 0; i < indentlevel; i++) {
			sprintf(cptr, "\t");
			gotoend(&cptr);
		}
		sprintf(cptr, "\n");
		gotoend(&cptr);
		for (int i = 0; i < indentlevel; i++) {
			sprintf(cptr, "\t");
			gotoend(&cptr);
		}
		sprintf(cptr, "%s *tape = (%s *)calloc(%d, sizeof(%s));\n",
			DECLTYPE_STR, DECLTYPE_STR, this->maxaddr - this->minaddr + 1, DECLTYPE_STR);
		gotoend(&cptr);
		for (int i = 0; i < indentlevel; i++) {
			sprintf(cptr, "\t");
			gotoend(&cptr);
		}
		sprintf(cptr, "ptrdiff_t i = 0;\n");
		gotoend(&cptr);
		sprintf(cptr, "\n");
		gotoend(&cptr);

		ptrdiff_t offset = 0;
		// Translates brainfuck codes to C code.
		// Consecutive + and -, as well as > and <, will be congealed into a single C statement.
		while (*bfptr != 0) {

			switch (*bfptr) {

			case '>': case '<': {

				storage_t totaloffset = 0;

				const char *bfsearchptr = bfptr;
				// Collates the total offset by consecutive > and < characters until an intervening character is reached.
				while (true) {
					if (*bfsearchptr == '>') {
						totaloffset++;
					} else if (*bfsearchptr == '<') {
						totaloffset--;
					} else {
						break;
					}
					bfsearchptr++;
				}
				// Translates code only if total pointer offset is not zero.
				for (int i = 0; i < indentlevel; i++) {
					sprintf(cptr, "\t");
					gotoend(&cptr);
				}
				if (totaloffset > 0) {
					if (totaloffset == 1) {
						sprintf(cptr, "i++;");
						gotoend(&cptr);
					}  else {
						sprintf(cptr, "i += %d;", totaloffset);
						gotoend(&cptr);
					}
				} else if (totaloffset < 0) {
					if (totaloffset == -1) {
						sprintf(cptr, "i--;");
						gotoend(&cptr);
					} else {
						sprintf(cptr, "i -= %d;", -totaloffset);
						gotoend(&cptr);
					}
				}
				// The brainfuck code usually consists of pairs of ptr movement and ptr value assignment characters. To improve readability, one pair of pointer movement and assignment characters will be put on the same line.
				if (*bfsearchptr == '+' || *bfsearchptr == '-') {
					sprintf(cptr, " ");
					gotoend(&cptr);
				} else {
					sprintf(cptr, "\n");
					gotoend(&cptr);
				}
				bfptr = bfsearchptr;
				break;
			}

			case '+': case '-': {

				storage_t totalchange = 0;

				const char *bfsearchptr = bfptr;
				// Collates the total change by consecutive + and - characters until an intervening character is reached.
				while (true) {
					if (*bfsearchptr == '+') {
						totalchange++;
					} else if (*bfsearchptr == '-') {
						totalchange--;
					} else {
						break;
					}
					bfsearchptr++;
				}
				// Translates code only if total change is not zero.
				// Determines if the value assignment statement is preceded by a pointer movement statement, if yes, no indentation is printed into the C code string.
				if (bfptr == bfcode ||
					(bfptr[-1] != '>' && bfptr[-1] != '<')) {
					for (int i = 0; i < indentlevel; i++) {
						sprintf(cptr, "\t");
						gotoend(&cptr);
					}
				}
				if (totalchange > 0) {
					if (totalchange == 1) {
						sprintf(cptr, "tape[i]++;\n");
						gotoend(&cptr);
					} else {
						sprintf(cptr, "tape[i] += %d;\n", totalchange);
						gotoend(&cptr);
					}
				} else if (totalchange < 0) {
					if (totalchange == -1) {
						sprintf(cptr, "tape[i]--;\n");
						gotoend(&cptr);
					} else {
						sprintf(cptr, "tape[i] -= %d;\n", -totalchange);
						gotoend(&cptr);
					}
				}
				bfptr = bfsearchptr;
				break;
			}

			case '.': {
				for (int i = 0; i < indentlevel; i++) {
					sprintf(cptr, "\t");
					gotoend(&cptr);
				}
				strcpy(cptr, "printf(\"%%c\", tape[i]);\n");
				gotoend(&cptr);
				bfptr++;
				break;
			}

			case ',': {
				for (int i = 0; i < indentlevel; i++) {
					sprintf(cptr, "\t");
					gotoend(&cptr);
				}
				sprintf(cptr, "tape[i] = _getch();\n");
				gotoend(&cptr);
				bfptr++;
				break;
			}

			case '[': {
				for (int i = 0; i < indentlevel; i++) {
					sprintf(cptr, "\t");
					gotoend(&cptr);
				}
				sprintf(cptr, "while (tape[i] != 0) {\n");
				indentlevel++;
				gotoend(&cptr);
				bfptr++;
				break;
			}

			case ']': {
				indentlevel--;
				for (int i = 0; i < indentlevel; i++) {
					sprintf(cptr, "\t");
					gotoend(&cptr);
				}
				sprintf(cptr, "}\n");
				gotoend(&cptr);
				bfptr++;
				break;
			}

			default: {
				bfptr++;
			}

			}

		}

		// Appends the ending for the C program.
		for (int i = 0; i < indentlevel; i++) {
			sprintf(cptr, "\t");
			gotoend(&cptr);
		}
		sprintf(cptr, "\n");
		gotoend(&cptr);
		for (int i = 0; i < indentlevel; i++) {
			sprintf(cptr, "\t");
			gotoend(&cptr);
		}
		sprintf(cptr, "free(tape);\n");
		gotoend(&cptr);
		for (int i = 0; i < indentlevel; i++) {
			sprintf(cptr, "\t");
			gotoend(&cptr);
		}
		sprintf(cptr, "_getch();\n");
		gotoend(&cptr);
		for (int i = 0; i < indentlevel; i++) {
			sprintf(cptr, "\t");
			gotoend(&cptr);
		}
		sprintf(cptr, "\n");
		gotoend(&cptr);
		for (int i = 0; i < indentlevel; i++) {
			sprintf(cptr, "\t");
			gotoend(&cptr);
		}
		sprintf(cptr, "return 0;\n");
		gotoend(&cptr);
		for (int i = 0; i < indentlevel; i++) {
			sprintf(cptr, "\t");
			gotoend(&cptr);
		}
		sprintf(cptr, "\n");
		gotoend(&cptr);
		indentlevel--;
		for (int i = 0; i < indentlevel; i++) {
			sprintf(cptr, "\t");
			gotoend(&cptr);
		}
		sprintf(cptr, "}\n");
		gotoend(&cptr);

		// Assess if there is any errors in the indentation, i.e. errors with unenclosed loops.
		return indentlevel == 0 ? 0 : 1;

	}

	// Manually resets the internal state of the brainfuck environment, where all storage units will be reinitialized to zero and the pointer to the start position of the memory block.
	void reset() {
		for (storage_t *resetptr = this->minaddr; resetptr <= this->maxaddr; resetptr++) {
			*resetptr = 0;
		}
		this->ptr = this->minaddr;
	}

	// Determines if the current pointer position is out of range.
	bool ptroutofrange() {
		return this->ptr < this->minaddr || this->ptr > this->maxaddr;
	}

};