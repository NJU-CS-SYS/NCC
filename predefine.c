// The predefined variables and functions.
// Previously, these functions are independent files.
// Now, they are functions embeded in the NCC.

#include <stdio.h>
#include <string.h>
#include <assert.h>

void predefined_data(FILE *fp){
    fprintf(fp, ".data\n");
    fprintf(fp, ".globl  _prompt\n");
    fprintf(fp, "_prompt:\n");
    fprintf(fp, "  .asciz \"Enter an integer:\"\n");
    fprintf(fp, ".globl  _ret\n");
    fprintf(fp, "_ret:\n");
    fprintf(fp, "  .asciz \"\\n\"\n");
}

void predefined_text(FILE *fp){
    fprintf(fp, ".text\n");
    fprintf(fp, ".globl read\n");
    fprintf(fp, "read:\n");
    fprintf(fp, "  movl $66, %%eax\n");
    fprintf(fp, "  movl $_prompt, %%ebx\n");
    fprintf(fp, "  int $0x80\n");
    fprintf(fp, "  ret\n");
    fprintf(fp, ".globl write\n");
    fprintf(fp, "write:\n");
    fprintf(fp, "  movl %%eax, %%ebx\n");
    fprintf(fp, "  movl $67, %%eax\n");
    fprintf(fp, "  movl $_ret, %%ecx\n");
    fprintf(fp, "  int $0x80\n");
    fprintf(fp, "  ret\n");
    fprintf(fp, ".globl writec\n");
    fprintf(fp, "writec:\n");
    fprintf(fp, "  movl %%eax, %%ebx\n");
    fprintf(fp, "  movl $68, %%eax\n");
    fprintf(fp, "  movl $_ret, %%ecx\n");
    fprintf(fp, "  int $0x80\n");
    fprintf(fp, "  ret\n");
    fprintf(fp, ".globl main\n");
}

