// EECS 370 Lab 1
// MODIFY AND SUBMIT THIS FILE
#include "lab1.h"



/*  Write a function int extract(int) that extracts bits 7 
    through 4 of the given integer a
    Example:
    extract(0x2020) returns 0b0010
*/
int extract(int a){
    //TODO: Implement this function
    return (a >> 4) & 0xF;
}