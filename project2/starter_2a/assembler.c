/**
 * Project 2
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000
#define MAXLINES 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static void checkForBlankLinesInCode(FILE *inFilePtr);
static void checkForInstructionsAfterFill(FILE *inFilePtr);
static inline int isNumber(char *);
static inline void printHexToFile(FILE *, int);

static struct {
    char name[MAXLINELENGTH];
    int address;
} labelTable[MAXLINES];
static int numLabels = 0;

// Global symbolic addresses that are used but never defined in this file.
static char undefinedGlobals[MAXLINES][MAXLINELENGTH];
static int numUndefinedGlobals = 0;

static struct {
    int offset;
    char opcode[MAXLINELENGTH];
    char label[MAXLINELENGTH];
} relocationTable[MAXLINES];
static int numRelocations = 0;

static int machineCode[MAXLINES];
static int numText = 0;
static int numData = 0;

// Global labels start with a capital letter; local labels start with a lowercase one.
static int isGlobalLabel(char *name) {
    // isupper only promises "nonzero" (glibc returns 256), so normalize to 0/1
    // because this result is also summed when counting symbols.
    return isupper((unsigned char)name[0]) != 0;
}

// Returns the address of a label, or -1 if it isn't defined.
static int lookupLabel(char *name) {
    for (int i = 0; i < numLabels; i++) {
        if (!strcmp(labelTable[i].name, name)) {
            return labelTable[i].address;
        }
    }
    return -1;
}

static void defineLabel(char *name, int address) {
    if (lookupLabel(name) != -1) {
        printf("error: duplicate definition of label %s\n", name);
        exit(1);
    }
    strcpy(labelTable[numLabels].name, name);
    labelTable[numLabels].address = address;
    numLabels++;
}

static void addUndefinedGlobal(char *name) {
    for (int i = 0; i < numUndefinedGlobals; i++) {
        if (!strcmp(undefinedGlobals[i], name)) {
            return;
        }
    }
    strcpy(undefinedGlobals[numUndefinedGlobals++], name);
}

static void addRelocation(int address, char *opcode, char *label) {
    // Offsets are relative to the start of the section the use appears in.
    relocationTable[numRelocations].offset = address < numText ? address : address - numText;
    strcpy(relocationTable[numRelocations].opcode, opcode);
    strcpy(relocationTable[numRelocations].label, label);
    numRelocations++;
}

// Resolves a symbolic address. Undefined global labels resolve to 0 (unless
// allowUndefinedGlobal is false, e.g. for beq); undefined local labels are errors.
static int resolveLabel(char *name, int allowUndefinedGlobal) {
    int address = lookupLabel(name);
    if (address != -1) {
        return address;
    }
    if (isGlobalLabel(name) && allowUndefinedGlobal) {
        addUndefinedGlobal(name);
        return 0;
    }
    printf("error: undefined label %s\n", name);
    exit(1);
}

// Validates a register argument and returns its value.
static int parseRegister(char *arg) {
    if (!isNumber(arg)) {
        printf("error: non-integer register argument '%s'\n", arg);
        exit(1);
    }
    int reg = atoi(arg);
    if (reg < 0 || reg > 7) {
        printf("error: register %d out of range [0,7]\n", reg);
        exit(1);
    }
    return reg;
}

// Resolves field2 of an I-type instruction into a 16-bit offsetField.
// Symbolic lw/sw offsets get a relocation entry; beq offsets are PC-relative and don't.
static int parseOffset(char *arg, int pc, char *opcode) {
    int offset;
    int isBeq = !strcmp(opcode, "beq") || !strcmp(opcode, "b");
    if (isNumber(arg)) {
        offset = atoi(arg);
    } else if (isBeq) {
        offset = resolveLabel(arg, 0) - (pc + 1);
    } else {
        offset = resolveLabel(arg, 1);
        addRelocation(pc, opcode, arg);
    }
    if (offset < -32768 || offset > 32767) {
        printf("error: offsetField %d does not fit in 16 bits\n", offset);
        exit(1);
    }
    return offset & 0xFFFF;
}

// Resolves the single field of a .fill directive.
static int parseFill(char *arg, int address) {
    if (isNumber(arg)) {
        return atoi(arg);
    }
    int value = resolveLabel(arg, 1);
    addRelocation(address, ".fill", arg);
    return value;
}

int main(int argc, char **argv) {
    char *inFileStr, *outFileStr;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileStr = argv[1];
    outFileStr = argv[2];

    inFilePtr = fopen(inFileStr, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileStr);
        exit(1);
    }
    // Check for blank lines in the middle of the code.
    checkForBlankLinesInCode(inFilePtr);

    // Check for instructions after .fill directive.
    checkForInstructionsAfterFill(inFilePtr);

    // first pass: store the address of every label and size each section
    rewind(inFilePtr);
    for (int address = 0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); address++) {
        if (label[0] != '\0') {
            defineLabel(label, address);
        }
        if (!strcmp(opcode, ".fill")) {
            numData++;
        } else {
            numText++;
        }
    }

    // second pass: transform to machine code, collecting symbol and relocation info
    rewind(inFilePtr);
    for (int address = 0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); address++) {
        int word;
        int regA, regB, destReg;

        if (!strcmp(opcode, "add") || !strcmp(opcode, "nor")) {
            int op = !strcmp(opcode, "add") ? 0 : 1;
            regA = parseRegister(arg0);
            regB = parseRegister(arg1);
            destReg = parseRegister(arg2);
            word = (op << 22) | (regA << 19) | (regB << 16) | destReg;
        } else if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw") || !strcmp(opcode, "beq")) {
            int op = !strcmp(opcode, "lw") ? 2 : (!strcmp(opcode, "sw") ? 3 : 4);
            regA = parseRegister(arg0);
            regB = parseRegister(arg1);
            int offset = parseOffset(arg2, address, opcode);
            word = (op << 22) | (regA << 19) | (regB << 16) | offset;
        } else if (!strcmp(opcode, "jalr")) {
            regA = parseRegister(arg0);
            regB = parseRegister(arg1);
            word = (5 << 22) | (regA << 19) | (regB << 16);
        } else if (!strcmp(opcode, "b")) {
            // pseudo-instruction: beq 0 0 <field0>
            word = (4 << 22) | parseOffset(arg0, address, opcode);
        } else if (!strcmp(opcode, "jump")) {
            // pseudo-instruction: jalr <field0> 0
            word = (5 << 22) | (parseRegister(arg0) << 19);
        } else if (!strcmp(opcode, "halt")) {
            word = 6 << 22;
        } else if (!strcmp(opcode, "noop")) {
            word = 7 << 22;
        } else if (!strcmp(opcode, ".fill")) {
            word = parseFill(arg0, address);
        } else {
            printf("error: unrecognized opcode %s\n", opcode);
            exit(1);
        }

        machineCode[address] = word;
    }
    fclose(inFilePtr);

    // Only write the object file once the whole input is known to be error-free.
    outFilePtr = fopen(outFileStr, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileStr);
        exit(1);
    }

    int numSymbols = numUndefinedGlobals;
    for (int i = 0; i < numLabels; i++) {
        numSymbols += isGlobalLabel(labelTable[i].name);
    }

    // Header
    fprintf(outFilePtr, "%d %d %d %d\n", numText, numData, numSymbols, numRelocations);

    // Text and Data
    for (int address = 0; address < numText + numData; address++) {
        printHexToFile(outFilePtr, machineCode[address]);
    }

    // Symbol table: defined globals, then undefined globals
    for (int i = 0; i < numLabels; i++) {
        if (isGlobalLabel(labelTable[i].name)) {
            int address = labelTable[i].address;
            if (address < numText) {
                fprintf(outFilePtr, "%s T %d\n", labelTable[i].name, address);
            } else {
                fprintf(outFilePtr, "%s D %d\n", labelTable[i].name, address - numText);
            }
        }
    }
    for (int i = 0; i < numUndefinedGlobals; i++) {
        fprintf(outFilePtr, "%s U 0\n", undefinedGlobals[i]);
    }

    // Relocation table
    for (int i = 0; i < numRelocations; i++) {
        fprintf(outFilePtr, "%d %s %s\n", relocationTable[i].offset,
            relocationTable[i].opcode, relocationTable[i].label);
    }

    fclose(outFilePtr);
    return(0);
}

/*
* NOTE: The code defined below is not to be modifed as it is implemented correctly.
*/

// Returns non-zero if the line contains only whitespace.
int lineIsBlank(char *line) {
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for(int line_idx=0; line_idx < strlen(line); ++line_idx) {
        int line_char_is_whitespace = 0;
        for(int whitespace_idx = 0; whitespace_idx < 4; ++whitespace_idx) {
            if(line[line_idx] == whitespace[whitespace_idx]) {
                line_char_is_whitespace = 1;
                break;
            }
        }
        if(!line_char_is_whitespace) {
            nonempty_line = 1;
            break;
        }
    }
    return !nonempty_line;
}

// Exits 2 if file contains an empty line anywhere other than at the end of the file.
// Note calling this function rewinds inFilePtr.
static void checkForBlankLinesInCode(FILE *inFilePtr) {
    char line[MAXLINELENGTH];
    int blank_line_encountered = 0;
    int address_of_blank_line = 0;
    rewind(inFilePtr);

    for(int address = 0; fgets(line, MAXLINELENGTH, inFilePtr) != NULL; ++address) {
        // Check for line too long
        if (strlen(line) >= MAXLINELENGTH-1) {
            printf("error: line too long\n");
            exit(1);
        }

        // Check for blank line.
        if(lineIsBlank(line)) {
            if(!blank_line_encountered) {
                blank_line_encountered = 1;
                address_of_blank_line = address;
            }
        } else {
            if(blank_line_encountered) {
                printf("Invalid Assembly: Empty line at address %d\n", address_of_blank_line);
                exit(2);
            }
        }
    }
    rewind(inFilePtr);
}

// Exits 2 if file contains a non-.fill instruction after the first .fill directive.
// Note calling this function rewinds inFilePtr.
static void checkForInstructionsAfterFill(FILE *inFilePtr) {
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
         arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    int fill_section_started = 0;
    int address_of_first_fill = 0;
    rewind(inFilePtr);

    for(int address = 0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); ++address) {
        // Check if we've encountered a .fill directive
        if (!strcmp(opcode, ".fill")) {
            if (!fill_section_started) {
                fill_section_started = 1;
                address_of_first_fill = address;
            }
        } else {
            // Non-.fill instruction found
            if (fill_section_started) {
                printf("Invalid Assembly: non-.fill instruction \"%s\" at address %d after .fill at address %d\n", 
                       opcode, address, address_of_first_fill);
                exit(2);
            }
        }
    }
    rewind(inFilePtr);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 */
int readAndParse(FILE *inFilePtr, char *label,
	char *opcode, char *arg0, char *arg1, char *arg2) {

    char line[MAXLINELENGTH];
    char *ptr = line;

    // delete prior values
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    // read the line from the assembly-language file
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
		// reached end of file
        return(0);
    }

    // check for line too long
    if (strlen(line) >= MAXLINELENGTH-1) {
		printf("error: line too long\n");
		exit(1);
    }

    // Ignore blank lines at the end of the file.
    if(lineIsBlank(line)) {
        return 0;
    }

    // is there a label?
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
		// successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    // Parse the rest of the line.  Would be nice to have real regular expressions, but scanf will suffice.
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

static inline int
isNumber(char *string)
{
    int num;
    char c;
    return((sscanf(string, "%d%c",&num, &c)) == 1);
}

// Prints a machine code word in the proper hex format to the file
static inline void 
printHexToFile(FILE *outFilePtr, int word) {
    fprintf(outFilePtr, "0x%08X\n", word);
}
