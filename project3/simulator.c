/*
 * EECS 370, University of Michigan, Fall 2023
 * Project 3: LC-2K Pipeline Simulator
 * Instructions are found in the project spec: https://eecs370.github.io/project_3_spec/
 * Make sure NOT to modify printState or any of the associated functions
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Machine Definitions
#define NUMMEMORY 65536 // maximum number of data words in memory
#define NUMREGS 8 // number of machine registers

#define ADD 0
#define NOR 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 // will not implemented for Project 3
#define HALT 6
#define NOOP 7

const char* opcode_to_str_map[] = {
    "add",
    "nor",
    "lw",
    "sw",
    "beq",
    "jalr",
    "halt",
    "noop"
};

#define NOOPINSTR (NOOP << 22)

typedef struct IFIDStruct {
    int instr;
	int pcPlus1;
} IFIDType;

typedef struct IDEXStruct {
    int instr;
	int pcPlus1;
	int valA;
	int valB;
	int offset;
} IDEXType;

typedef struct EXMEMStruct {
    int instr;
	int branchTarget;
    int eq;
	int aluResult;
	int valB;
} EXMEMType;

typedef struct MEMWBStruct {
    int instr;
	int writeData;
} MEMWBType;

typedef struct WBENDStruct {
    int instr;
	int writeData;
} WBENDType;

typedef struct stateStruct {
    unsigned int numMemory;
    unsigned int cycles; // number of cycles run so far
	int pc;
	int instrMem[NUMMEMORY];
	int dataMem[NUMMEMORY];
	int reg[NUMREGS];
	IFIDType IFID;
	IDEXType IDEX;
	EXMEMType EXMEM;
	MEMWBType MEMWB;
	WBENDType WBEND;
} stateType;

static inline int opcode(int instruction) {
    return instruction>>22;
}

static inline int field0(int instruction) {
    return (instruction>>19) & 0x7;
}

static inline int field1(int instruction) {
    return (instruction>>16) & 0x7;
}

static inline int field2(int instruction) {
    return instruction & 0xFFFF;
}

// convert a 16-bit number into a 32-bit Linux integer
static inline int convertNum(int num) {
    return num - ( (num & (1<<15)) ? 1<<16 : 0 );
}

void printState(stateType*);
void printInstruction(int);
void readMachineCode(stateType*, char*);


int main(int argc, char *argv[]) {

    /* Declare state and newState.
       Note these have static lifetime so that instrMem and
       dataMem are not allocated on the stack. */

    static stateType state, newState;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    readMachineCode(&state, argv[1]);

    /* ------------ Initialize State ------------ */
    state.cycles = 0;
    state.pc = 0;
    for (int i = 0; i < NUMREGS; ++i) {
        state.reg[i] = 0;
    }
    state.IFID.instr = NOOPINSTR;
    state.IDEX.instr = NOOPINSTR;
    state.EXMEM.instr = NOOPINSTR;
    state.MEMWB.instr = NOOPINSTR;
    state.WBEND.instr = NOOPINSTR;
    /* ------------------- END ------------------ */

    newState = state;

    while (opcode(state.MEMWB.instr) != HALT) {
        printState(&state);

        newState.cycles += 1;

        /* ---------------------- IF stage --------------------- */
        newState.IFID.instr = state.instrMem[state.pc];
        newState.IFID.pcPlus1 = state.pc + 1;
        newState.pc = state.pc + 1;

        /* ---------------------- ID stage --------------------- */
        int idInstr = state.IFID.instr;
        newState.IDEX.instr = idInstr;
        newState.IDEX.pcPlus1 = state.IFID.pcPlus1;
        newState.IDEX.valA = state.reg[field0(idInstr)];
        newState.IDEX.valB = state.reg[field1(idInstr)];
        newState.IDEX.offset = convertNum(field2(idInstr));

        // load-use hazard: lw in EX, dependent instruction in ID -> stall one cycle
        if (opcode(state.IDEX.instr) == LW) {
            int lwDest = field1(state.IDEX.instr);
            int idOp = opcode(idInstr);
            int readsA = idOp == ADD || idOp == NOR || idOp == LW || idOp == SW || idOp == BEQ;
            int readsB = idOp == ADD || idOp == NOR || idOp == SW || idOp == BEQ;
            if (lwDest != 0 && ((readsA && field0(idInstr) == lwDest) ||
                                (readsB && field1(idInstr) == lwDest))) {
                newState.IDEX.instr = NOOPINSTR;  // bubble into EX
                newState.IFID = state.IFID;       // hold instruction in ID
                newState.pc = state.pc;           // hold fetch
            }
        }

        /* ---------------------- EX stage --------------------- */
        int exInstr = state.IDEX.instr;
        int exA = state.IDEX.valA;
        int exB = state.IDEX.valB;
        int exRegA = field0(exInstr);
        int exRegB = field1(exInstr);

        // forwarding: check oldest (WBEND) first so newer stages overwrite it
        int fwdInstr[3] = {state.WBEND.instr, state.MEMWB.instr, state.EXMEM.instr};
        int fwdData[3] = {state.WBEND.writeData, state.MEMWB.writeData, state.EXMEM.aluResult};
        for (int i = 0; i < 3; ++i) {
            int op = opcode(fwdInstr[i]);
            int dest;
            if (op == ADD || op == NOR) {
                dest = field2(fwdInstr[i]) & 0x7;
            } else if (op == LW && i != 2) { // lw in EXMEM has no data yet (stall prevents this case)
                dest = field1(fwdInstr[i]);
            } else {
                continue;
            }
            if (dest == 0) {
                continue; // reg 0 is always 0
            }
            if (dest == exRegA) {
                exA = fwdData[i];
            }
            if (dest == exRegB) {
                exB = fwdData[i];
            }
        }

        newState.EXMEM.instr = exInstr;
        newState.EXMEM.branchTarget = state.IDEX.pcPlus1 + state.IDEX.offset;
        newState.EXMEM.eq = (exA == exB);
        newState.EXMEM.valB = exB;
        switch (opcode(exInstr)) {
            case ADD:
                newState.EXMEM.aluResult = exA + exB;
                break;
            case NOR:
                newState.EXMEM.aluResult = ~(exA | exB);
                break;
            case LW:
            case SW:
                newState.EXMEM.aluResult = exA + state.IDEX.offset;
                break;
            default:
                newState.EXMEM.aluResult = 0;
                break;
        }

        /* --------------------- MEM stage --------------------- */
        int memInstr = state.EXMEM.instr;
        newState.MEMWB.instr = memInstr;
        newState.MEMWB.writeData = state.EXMEM.aluResult;
        if (opcode(memInstr) == LW) {
            newState.MEMWB.writeData = state.dataMem[state.EXMEM.aluResult];
        } else if (opcode(memInstr) == SW) {
            newState.dataMem[state.EXMEM.aluResult] = state.EXMEM.valB;
        } else if (opcode(memInstr) == BEQ && state.EXMEM.eq) {
            // predicted not-taken was wrong: squash IF, ID, EX and redirect
            newState.pc = state.EXMEM.branchTarget;
            newState.IFID.instr = NOOPINSTR;
            newState.IDEX.instr = NOOPINSTR;
            newState.EXMEM.instr = NOOPINSTR;
        }

        /* ---------------------- WB stage --------------------- */
        int wbInstr = state.MEMWB.instr;
        newState.WBEND.instr = wbInstr;
        newState.WBEND.writeData = state.MEMWB.writeData;
        if (opcode(wbInstr) == ADD || opcode(wbInstr) == NOR) {
            int dest = field2(wbInstr) & 0x7;
            if (dest != 0) {
                newState.reg[dest] = state.MEMWB.writeData;
            }
        } else if (opcode(wbInstr) == LW) {
            int dest = field1(wbInstr);
            if (dest != 0) {
                newState.reg[dest] = state.MEMWB.writeData;
            }
        }

        /* ------------------------ END ------------------------ */
        state = newState; /* this is the last statement before end of the loop. It marks the end
        of the cycle and updates the current state with the values calculated in this cycle */
    }
    printf("Machine halted\n");
    printf("Total of %d cycles executed\n", state.cycles);
    printf("Final state of machine:\n");
    printState(&state);
}

/*
* DO NOT MODIFY ANY OF THE CODE BELOW.
*/

void printInstruction(int instr) {
    const char* instr_opcode_str;
    int instr_opcode = opcode(instr);
    if(ADD <= instr_opcode && instr_opcode <= NOOP) {
        instr_opcode_str = opcode_to_str_map[instr_opcode];
    }

    switch (instr_opcode) {
        case ADD:
        case NOR:
        case LW:
        case SW:
        case BEQ:
            printf("%s %d %d %d", instr_opcode_str, field0(instr), field1(instr), convertNum(field2(instr)));
            break;
        case JALR:
            printf("%s %d %d", instr_opcode_str, field0(instr), field1(instr));
            break;
        case HALT:
        case NOOP:
            printf("%s", instr_opcode_str);
            break;
        default:
            printf(".fill %d", instr);
            return;
    }
}

void printState(stateType *statePtr) {
    printf("\n@@@\n");
    printf("state before cycle %d starts:\n", statePtr->cycles);
    printf("\tpc = %d\n", statePtr->pc);

    printf("\tdata memory:\n");
    for (int i=0; i<statePtr->numMemory; ++i) {
        printf("\t\tdataMem[ %d ] = 0x%08X\n", i, statePtr->dataMem[i]);
    }
    printf("\tregisters:\n");
    for (int i=0; i<NUMREGS; ++i) {
        printf("\t\treg[ %d ] = %d\n", i, statePtr->reg[i]);
    }

    // IF/ID
    printf("\tIF/ID pipeline register:\n");
    printf("\t\tinstruction = 0x%08X ( ", statePtr->IFID.instr);
    printInstruction(statePtr->IFID.instr);
    printf(" )\n");
    printf("\t\tpcPlus1 = %d", statePtr->IFID.pcPlus1);
    if(opcode(statePtr->IFID.instr) == NOOP){
        printf(" (Don't Care)");
    }
    printf("\n");
    
    // ID/EX
    int idexOp = opcode(statePtr->IDEX.instr);
    printf("\tID/EX pipeline register:\n");
    printf("\t\tinstruction = 0x%08X ( ", statePtr->IDEX.instr);
    printInstruction(statePtr->IDEX.instr);
    printf(" )\n");
    printf("\t\tpcPlus1 = %d", statePtr->IDEX.pcPlus1);
    if(idexOp == NOOP){
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\tvalA = %d", statePtr->IDEX.valA);
    if (idexOp >= HALT || idexOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\tvalB = %d", statePtr->IDEX.valB);
    if(idexOp == LW || idexOp > BEQ || idexOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\toffset = %d", statePtr->IDEX.offset);
    if (idexOp != LW && idexOp != SW && idexOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");

    // EX/MEM
    int exmemOp = opcode(statePtr->EXMEM.instr);
    printf("\tEX/MEM pipeline register:\n");
    printf("\t\tinstruction = 0x%08X ( ", statePtr->EXMEM.instr);
    printInstruction(statePtr->EXMEM.instr);
    printf(" )\n");
    printf("\t\tbranchTarget %d", statePtr->EXMEM.branchTarget);
    if (exmemOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\teq ? %s", (statePtr->EXMEM.eq ? "True" : "False"));
    if (exmemOp != BEQ) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\taluResult = %d", statePtr->EXMEM.aluResult);
    if (exmemOp > SW || exmemOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");
    printf("\t\tvalB = %d", statePtr->EXMEM.valB);
    if (exmemOp != SW) {
        printf(" (Don't Care)");
    }
    printf("\n");

    // MEM/WB
	int memwbOp = opcode(statePtr->MEMWB.instr);
    printf("\tMEM/WB pipeline register:\n");
    printf("\t\tinstruction = 0x%08X ( ", statePtr->MEMWB.instr);
    printInstruction(statePtr->MEMWB.instr);
    printf(" )\n");
    printf("\t\twriteData = %d", statePtr->MEMWB.writeData);
    if (memwbOp >= SW || memwbOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");     

    // WB/END
	int wbendOp = opcode(statePtr->WBEND.instr);
    printf("\tWB/END pipeline register:\n");
    printf("\t\tinstruction = 0x%08X ( ", statePtr->WBEND.instr);
    printInstruction(statePtr->WBEND.instr);
    printf(" )\n");
    printf("\t\twriteData = %d", statePtr->WBEND.writeData);
    if (wbendOp >= SW || wbendOp < 0) {
        printf(" (Don't Care)");
    }
    printf("\n");

    printf("end state\n");
    fflush(stdout);
}

// File
#define MAXLINELENGTH 1000 // MAXLINELENGTH is the max number of characters we read

void readMachineCode(stateType *state, char* filename) {
    char line[MAXLINELENGTH];
    FILE *filePtr = fopen(filename, "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", filename);
        exit(1);
    }

    printf("instruction memory:\n");
    for (state->numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; ++state->numMemory) {
        if (sscanf(line, "%x", state->instrMem+state->numMemory) != 1) {
            printf("error in reading address %d\n", state->numMemory);
            exit(1);
        }
        printf("\tinstrMem[ %d ] = 0x%08X ( ", state->numMemory, 
            state->instrMem[state->numMemory]);
        printInstruction(state->dataMem[state->numMemory] = state->instrMem[state->numMemory]);
        printf(" )\n");
    }
}
