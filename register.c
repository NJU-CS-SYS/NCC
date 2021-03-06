//
// Created by whz on 15-12-24.
//

#include "register.h"
#include "asm.h"
#include "lib.h"
#include "ir.h"
#include "operand.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


extern FILE *asm_file;


const char *reg_s[] = {
    "%eax", "%ebx", "%ecx", "%edx",
    "%ebp", "%esp", "%esi", "%edi"
};

#define NR_REG (sizeof(reg_s) / sizeof(reg_s[0]))


#define NR_SAVE 4

#define MAX_VAR 4096


int sp_offset = 0;  // Always positive, [-n]($fp) == [offset - n]($sp) where offset == $fp - $sp


Operand ope_in_reg[NR_REG];  // Record which register stores which operand, null if none.


int dirty[NR_REG];  // True if $sx is written


void set_dirty(int index)
{
    if(index < GLOBAL_BASE){
        dirty[index] = 1;
    }
}

const char *reg_to_s(int index)
{
    if(index < GLOBAL_BASE){
        return reg_s[index];
    } else {
        // global variable name output.
        index = index - GLOBAL_BASE;
        assert(index < global_count);
        return global_var_buf[index].rs->name;
    }
}


int get_reg(int start, int end)  // [start, end]
{
    int victim = MAX_LINE;           // The one to be replaced
    int victim_next_use = -1;  // The limit of instruction buffer

    int i;  // Need to use the break index

    for (i = start; i <= end; i++) {
        Operand ope = ope_in_reg[i];

        if (ope == NULL) {  // An empty register
            break;
        }
        else if (!ope->is_using && victim_next_use < ope->next_use) {
            victim = i;
            victim_next_use = ope->next_use;
        }
    }

    if (i <= end) {  // Find empty register
        return i;
    }
    else {
        TEST(start <= victim && victim <= end && ope_in_reg[victim], "Victim should be updated");
        Operand vic = ope_in_reg[victim];
        if (vic->next_use != MAX_LINE || vic->liveness) {
            /*if (vic->type == OPE_TEMP) {
                WARN("Back up temporary variable");
            }
            else*/ if (vic->type == OPE_GLOBAL){
                // Back up global variable
                emit_asm(movl, "%s, %s         # Back up victim", reg_s[victim], print_operand(vic));
            } 
            else {
                emit_asm(movl, "%s, %d(%%esp)  # Back up victim", reg_s[victim], sp_offset - ope_in_reg[victim]->address);
            }
        }
        ope_in_reg[victim] = NULL;
        return victim;
    }
}

int get_global(Operand ope)  // Get the global varible slot in global_var_buf
{
    // Add global varible index search.
    for(int i=0; i < global_count; i++){
        if(strcmp(ope->name, global_var_buf[i].rs->name) == 0){
            return GLOBAL_BASE + i;
        }
    }
    PANIC("Cannot find global varible %s", ope->name);
    return GLOBAL_BASE;
}


//
// Allocate a register to the given register.
//
// Registers are selected by the follow priority:
//   1. Empty register
//   2. Register with a value that is the least currently needed.
//

void remove_value(Operand ope)
{
    for (int i = 0; i <= NR_REG; i++) {
        if (ope_in_reg[i] == ope) {
            ope_in_reg[i] = NULL;
        }
    }
}


int allocate(Operand ope)
{
    TEST(ope, "Operand is null");

    int reg;
    
    remove_value(ope);  // Must remove ope's value stored in register, otherwise `ensure' will return old one.

    // Select a suitable register group
    switch (ope->type) {
    case OPE_VAR:
    case OPE_GLOBAL:
    case OPE_BOOL:
        reg = get_reg(EAX, EDX);
        break;
    case OPE_TEMP:
    case OPE_ADDR:
    case OPE_INTEGER:
    case OPE_CHAR:
        reg = get_reg(EAX, EDX);
        break;
    default:
        PANIC("Unexpected operand type when allocating registers");
        reg = get_reg(EAX, EDX);
    }

    ope_in_reg[reg] = ope;
    LOG("Allocate %s to register %s", print_operand(ope), reg_to_s(reg));

    return reg;
}


//
// Ensure an operand's value is in a register,
// otherwise emit a load instruction
//

int ensure(Operand ope)
{
    TEST(ope, "Operand is null");

/*
    if(ope->type == OPE_GLOBAL){
        LOG("Find global variable %s", print_operand(ope));
        return get_global(ope);
    }
*/
    for (int i = 0; i < NR_REG; i++) {
        if (ope_in_reg[i] && cmp_operand(ope, ope_in_reg[i])) {
            LOG("Find %s at %s", print_operand(ope), reg_to_s(i));
            return i;
        }
    }

    int result = allocate(ope);  // The reg name string to be printed.

    if (is_const(ope)) {
        emit_asm(movl, "$%d, %s", ope->integer, reg_s[result]); // Jump '#' required by ir
    }
    else if(ope->type == OPE_GLOBAL){
        // ensure global_var
        emit_asm(movl, "%s, %s  # Move global variable %s into register %s",
                print_operand(ope), reg_s[result], print_operand(ope), reg_to_s(result));
    }
    else {
        emit_asm(movl, "%d(%%esp), %s  # sp_offset %d addr %d",
                sp_offset - ope->address, reg_s[result], sp_offset, ope->address);
    }

    return result;
}


//
// Push variables and bools onto stak
//
// Address and temp are live only in one basic block
//

void push_all()
{
    for (int i = 0; i < NR_REG; i++) {
        Operand ope = ope_in_reg[i];
        if (ope != NULL && dirty[i] && (ope->type == OPE_GLOBAL)){
            // Backup global variables.
            emit_asm(movl, "%s, %s         # Push %s", 
                    reg_s[i], print_operand(ope), print_operand(ope));
        }
        else if (ope != NULL && dirty[i] && !is_const(ope) &&(ope->next_use != MAX_LINE || ope->liveness)) {
            // Use next_use to avoid store dead temporary variables.
            // Use liveness to promise that user-defined variables are backed up.
            emit_asm(movl, "%s, %d(%%esp)  # push %s", reg_s[i], sp_offset - ope->address, print_operand(ope));
        }
    }
}


//
// Clear the operands' value stored in registers
//
// This function should be called at the end of a basic block.
//

void clear_reg_state()
{
    memset(ope_in_reg, 0, sizeof(ope_in_reg));
    memset(dirty, 0, sizeof(dirty));
}

