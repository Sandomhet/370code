/*
 * Project 1
 * EECS 370 LC-2K Instruction-level simulator
 *
 * Make sure to NOT modify printState or any of the associated functions
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//DO NOT CHANGE THE FOLLOWING DEFINITIONS 

// Machine Definitions
#define MEMORYSIZE 65536 /* maximum number of words in memory (maximum number of lines in a given file)*/
#define NUMREGS 8 /*total number of machine registers [0,7]*/

// File Definitions
#define MAXLINELENGTH 1000 /* MAXLINELENGTH is the max number of characters we read */

// Opcodes, found in bits 24-22 of an instruction
#define ADD 0
#define NOR 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5
#define HALT 6
#define NOOP 7

typedef struct stateStruct {
    int pc;
    int mem[MEMORYSIZE];
    int reg[NUMREGS];
    int numMemory;
    int numInstructionsExecuted;
} stateType;

void printState(stateType *);

void printStats(stateType *);

static inline int convertNum(int32_t);

static int endsWith(char *, char *);

static inline int getOpcode(int);
static inline int getRegA(int);
static inline int getRegB(int);
static inline int getDestReg(int);
static inline int getOffset(int);


int main(int argc, char **argv)
{
    char line[MAXLINELENGTH];
    stateType state = {0};
    FILE *filePtr;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    if (!endsWith(argv[1], ".mc")) {
        printf("warning: machine code file does not end with .mc\n");
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s , please ensure you are providing the correct path", argv[1]);
        perror("fopen");
        exit(2);
    }

    // read the entire machine-code file into memory
    for (state.numMemory=0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++) {
		    if (state.numMemory >= MEMORYSIZE) {
			      fprintf(stderr, "exceeded memory size\n");
			      exit(2);
		    }
		    if (sscanf(line, "%x", state.mem+state.numMemory) != 1) {
			      fprintf(stderr, "error in reading address %d\n", state.numMemory);
			      exit(2);
		    }
            printf("mem[ %d ] 0x%08X\n", state.numMemory, state.mem[state.numMemory]);
    }

    // the pc and all registers start at 0
    state.pc = 0;
    state.numInstructionsExecuted = 0;
    for (int i = 0; i < NUMREGS; i++) {
        state.reg[i] = 0;
    }

    // execute instructions until the machine halts
    while (1) {
        printState(&state);

        int instr = state.mem[state.pc];
        int opcode = getOpcode(instr);
        int regA = getRegA(instr);
        int regB = getRegB(instr);
        int destReg = getDestReg(instr);
        int offset = getOffset(instr);

        state.pc++;
        state.numInstructionsExecuted++;

        switch (opcode) {
            case ADD:
                state.reg[destReg] = state.reg[regA] + state.reg[regB];
                break;
            case NOR:
                state.reg[destReg] = ~(state.reg[regA] | state.reg[regB]);
                break;
            case LW:
                state.reg[regB] = state.mem[state.reg[regA] + offset];
                break;
            case SW:
                state.mem[state.reg[regA] + offset] = state.reg[regB];
                break;
            case BEQ:
                if (state.reg[regA] == state.reg[regB]) {
                    state.pc += offset;
                }
                break;
            case JALR:
                state.reg[regB] = state.pc;
                state.reg[0] = 0;
                state.pc = state.reg[regA];
                break;
            case HALT:
                break;
            case NOOP:
                break;
            default:
                printf("error: unrecognized opcode %d in instruction 0x%08X at address %d\n",
                       opcode, instr, state.pc - 1);
                exit(1);
        }

        // register 0 is always 0, even if an instruction writes to it
        state.reg[0] = 0;

        if (opcode == HALT) {
            break;
        }
    }

    printStats(&state);
    printState(&state);

    fclose(filePtr);

    return(0);
}

/*
* DO NOT MODIFY ANY OF THE CODE BELOW. 
*/

void printState(stateType *statePtr) {
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] 0x%08X\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
	  for (i=0; i<NUMREGS; i++) {
	      printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
	  }
    printf("end state\n");
}

// convert a 16-bit number into a 32-bit Linux integer
static inline int convertNum(int num) 
{
    return num - ( (num & (1<<15)) ? 1<<16 : 0 );
}

/*
 * print end of run statistics like in the spec. **This is not required**,
 * but is helpful in debugging.
 * This should be called once a halt is reached.
 * DO NOT delete this function, or else it won't compile.
 * DO NOT print "@@@" or "end state" in this function
 */
void printStats(stateType *statePtr)
{
    printf("machine halted\n");
    printf("total of %d instructions executed\n", statePtr->numInstructionsExecuted);
    printf("final state of machine:\n");
}

// Returns 1 if string ends with substr, 0 otherwise
static int endsWith(char *string, char *substr)
{
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

/*
* Write any helper functions that you wish down here. 
*/

// bits 24-22 of an instruction
static inline int getOpcode(int instr)
{
    return (instr >> 22) & 0x7;
}

// bits 21-19 of an instruction
static inline int getRegA(int instr)
{
    return (instr >> 19) & 0x7;
}

// bits 18-16 of an instruction
static inline int getRegB(int instr)
{
    return (instr >> 16) & 0x7;
}

// bits 2-0 of an R-type instruction
static inline int getDestReg(int instr)
{
    return instr & 0x7;
}

// bits 15-0 of an I-type instruction, sign extended to 32 bits
static inline int getOffset(int instr)
{
    return convertNum(instr & 0xFFFF);
}
