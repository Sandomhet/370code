/**
 * Project 2
 * LC-2K Linker
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXSIZE 500
#define MAXLINELENGTH 1000
#define MAXFILES 6

static inline void printHexToFile(FILE *, int);
static int endsWith(char *, char *);

typedef struct FileData FileData;
typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct RelocationTableEntry RelocationTableEntry;
typedef struct CombinedFiles CombinedFiles;

static int lookupGlobal(CombinedFiles *, char *);
static int isDefinedInFile(FileData *, char *);

struct SymbolTableEntry {
	char label[7];
	char location;
	unsigned int offset;
};

struct RelocationTableEntry {
    unsigned int file;
	unsigned int offset;
	char inst[6];
	char label[7];
};

struct FileData {
	unsigned int textSize;
	unsigned int dataSize;
	unsigned int symbolTableSize;
	unsigned int relocationTableSize;
	unsigned int textStartingLine; // in final executable
	unsigned int dataStartingLine; // in final executable
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry symbolTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
};

struct CombinedFiles {
	unsigned int textSize;
	unsigned int dataSize;
	unsigned int symbolTableSize;
	unsigned int relocationTableSize;
	int text[MAXSIZE * MAXFILES];
	int data[MAXSIZE * MAXFILES];
	SymbolTableEntry symbolTable[MAXSIZE * MAXFILES];
	RelocationTableEntry relocTable[MAXSIZE * MAXFILES];
};

int main(int argc, char *argv[]) {
	char *inFileStr, *outFileStr;
	FILE *inFilePtr, *outFilePtr; 
	unsigned int i, j;

    if (argc <= 2 || argc > 8 ) {
        printf("error: usage: %s <MAIN-object-file> ... <object-file> ... <machine-code-file>, with at most 5 object files\n",
				argv[0]);
		exit(1);
	}

	outFileStr = argv[argc - 1];
    if (endsWith(outFileStr, ".obj")) {
        printf("error: no machine code file given\n");
        exit(1);
    }

	outFilePtr = fopen(outFileStr, "w");
	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileStr);
		exit(1);
	}

	FileData files[MAXFILES];

	// read in all files and combine into a "master" file
	for (i = 0; i < argc - 2; ++i) {
		inFileStr = argv[i+1];

		inFilePtr = fopen(inFileStr, "r");
		printf("opening %s\n", inFileStr);

		if (inFilePtr == NULL) {
			printf("error in opening %s\n", inFileStr);
			exit(1);
		}

		char line[MAXLINELENGTH];
		unsigned int textSize, dataSize, symbolTableSize, relocationTableSize;

		// parse first line of file
		fgets(line, MAXSIZE, inFilePtr);
		sscanf(line, "%d %d %d %d",
				&textSize, &dataSize, &symbolTableSize, &relocationTableSize);

		files[i].textSize = textSize;
		files[i].dataSize = dataSize;
		files[i].symbolTableSize = symbolTableSize;
		files[i].relocationTableSize = relocationTableSize;

		// read in text section
		int instr;
		for (j = 0; j < textSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			instr = strtol(line, NULL, 0);
			files[i].text[j] = instr;
		}

		// read in data section
		int data;
		for (j = 0; j < dataSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			data = strtol(line, NULL, 0);
			files[i].data[j] = data;
		}

		// read in the symbol table
		char label[7];
		char type;
		unsigned int addr;
		for (j = 0; j < symbolTableSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%s %c %d",
					label, &type, &addr);
			files[i].symbolTable[j].offset = addr;
			strcpy(files[i].symbolTable[j].label, label);
			files[i].symbolTable[j].location = type;
		}

		// read in relocation table
		char opcode[7];
		for (j = 0; j < relocationTableSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%d %s %s",
					&addr, opcode, label);
			files[i].relocTable[j].offset = addr;
			strcpy(files[i].relocTable[j].inst, opcode);
			strcpy(files[i].relocTable[j].label, label);
			files[i].relocTable[j].file	= i;
		}
		fclose(inFilePtr);
	} // end reading files

	unsigned int numFiles = argc - 2;

	// lay out the final executable: all text sections, then all data sections, then Stack
	CombinedFiles combined;
	combined.textSize = 0;
	combined.dataSize = 0;
	for (i = 0; i < numFiles; ++i) {
		files[i].textStartingLine = combined.textSize;
		combined.textSize += files[i].textSize;
	}
	for (i = 0; i < numFiles; ++i) {
		files[i].dataStartingLine = combined.textSize + combined.dataSize;
		combined.dataSize += files[i].dataSize;
	}
	unsigned int stackAddress = combined.textSize + combined.dataSize;

	// build the global symbol table from every defined (T/D) global label,
	// storing each label's final address in its offset field
	combined.symbolTableSize = 0;
	for (i = 0; i < numFiles; ++i) {
		for (j = 0; j < files[i].symbolTableSize; ++j) {
			SymbolTableEntry *symbol = &files[i].symbolTable[j];
			if (symbol->location == 'U') {
				continue;
			}
			if (!strcmp(symbol->label, "Stack")) {
				printf("error: Stack label defined by %s\n", argv[i+1]);
				exit(1);
			}
			if (lookupGlobal(&combined, symbol->label) != -1) {
				printf("error: duplicate definition of global label %s\n", symbol->label);
				exit(1);
			}
			SymbolTableEntry *global = &combined.symbolTable[combined.symbolTableSize++];
			strcpy(global->label, symbol->label);
			global->location = symbol->location;
			global->offset = (symbol->location == 'T' ? files[i].textStartingLine
					: files[i].dataStartingLine) + symbol->offset;
		}
	}

	// every global used but undefined in its file must be defined somewhere else
	for (i = 0; i < numFiles; ++i) {
		for (j = 0; j < files[i].symbolTableSize; ++j) {
			SymbolTableEntry *symbol = &files[i].symbolTable[j];
			if (symbol->location == 'U' && strcmp(symbol->label, "Stack")
					&& lookupGlobal(&combined, symbol->label) == -1) {
				printf("error: undefined global label %s\n", symbol->label);
				exit(1);
			}
		}
	}

	// fix every symbolic address listed in the relocation tables
	for (i = 0; i < numFiles; ++i) {
		FileData *file = &files[i];
		for (j = 0; j < file->relocationTableSize; ++j) {
			RelocationTableEntry *reloc = &file->relocTable[j];
			int isFill = !strcmp(reloc->inst, ".fill");
			int *word = isFill ? &file->data[reloc->offset] : &file->text[reloc->offset];
			// .fill holds the full address; lw/sw hold it in the 16-bit offsetField
			int oldAddress = isFill ? *word : (*word & 0xFFFF);
			int newAddress;

			if (isDefinedInFile(file, reloc->label)) {
				// local label, or a global defined in this same file: the old address
				// is relative to this file, so shift it by its section's new start
				if (oldAddress < (int)file->textSize) {
					newAddress = file->textStartingLine + oldAddress;
				} else {
					newAddress = file->dataStartingLine + (oldAddress - file->textSize);
				}
			} else if (!strcmp(reloc->label, "Stack")) {
				newAddress = oldAddress + stackAddress;
			} else {
				// undefined in this file, so it was assembled as 0 (plus any constant offset)
				newAddress = oldAddress + lookupGlobal(&combined, reloc->label);
			}

			if (isFill) {
				*word = newAddress;
			} else {
				*word = (*word & ~0xFFFF) | (newAddress & 0xFFFF);
			}
		}
	}

	// write the executable
	for (i = 0; i < numFiles; ++i) {
		for (j = 0; j < files[i].textSize; ++j) {
			printHexToFile(outFilePtr, files[i].text[j]);
		}
	}
	for (i = 0; i < numFiles; ++i) {
		for (j = 0; j < files[i].dataSize; ++j) {
			printHexToFile(outFilePtr, files[i].data[j]);
		}
	}
	fclose(outFilePtr);

	return 0;
} // main

// Returns the final address of a defined global label, or -1 if no file defines it.
static int
lookupGlobal(CombinedFiles *combined, char *label) {
	for (unsigned int i = 0; i < combined->symbolTableSize; ++i) {
		if (!strcmp(combined->symbolTable[i].label, label)) {
			return combined->symbolTable[i].offset;
		}
	}
	return -1;
}

// Returns 1 if label is defined inside file, 0 if it is an undefined global.
// Local labels must always be defined in their own file, so they never appear in the
// symbol table; only globals listed as 'U' are defined elsewhere.
static int
isDefinedInFile(FileData *file, char *label) {
	for (unsigned int i = 0; i < file->symbolTableSize; ++i) {
		if (!strcmp(file->symbolTable[i].label, label)) {
			return file->symbolTable[i].location != 'U';
		}
	}
	return 1;
}

// Prints a machine code word in the proper hex format to the file
static inline void 
printHexToFile(FILE *outFilePtr, int word) {
    fprintf(outFilePtr, "0x%08X\n", word);
}

// Returns 1 if string ends with substr, 0 otherwise
static int
endsWith(char *string, char *substr) {
    size_t stringLen = strlen(string);
    size_t substrLen = strlen(substr);
    if (stringLen < substrLen) {
        return 0; // string too short
    }
    char *stringEnd = string + stringLen - substrLen;
    if (strcmp(stringEnd, substr) == 0) {
        return 1;
    }
    return 0;
}
