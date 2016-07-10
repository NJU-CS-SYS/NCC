//
// Created by whz on 15-12-24.
//
// The module to translate intermediate representation to assembly code
// Architecture: MIPS32
//

#include "asm.h"
#include "register.h"
#include "ir.h"
#include "lib.h"
#include "operand.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>


FILE *asm_file  = NULL;  // Store the final assembly code

FILE *func_file = NULL;  // Store the temporary assembly code

Operand curr_func = NULL;


int nr_arg = 0;  // The number of arguments have been encounterded, referred when translating call


void gen_asm_label(IR *ir)
{
    fprintf(asm_file, "%s:\n", print_operand(ir->rs));
}


void gen_asm_func(IR *ir)
{
    fprintf(asm_file, "%s:\n", print_operand(ir->rs));
    // Spare stack space
    curr_func = ir->rs;
    sp_offset = curr_func->size;
    if(ir->rs->size != 0)
        emit_asm(addl, "$%d, %%esp  # only for variables, not records", -ir->rs->size);
    /* In x86, 'call' will save the return address itself.
    if (curr_func->has_subroutine) {
        emit_asm(movl, "%%eax, %d(%%esp)  # Save return address", sp_offset);
    }
    */
}


void gen_asm_assign(IR *ir)
{
    int src = ensure(ir->rs);
    int dst = allocate(ir->rd);
    set_dirty(dst);
    if(src != dst)
        emit_asm(movl, "%s, %s", reg_to_s(src), reg_to_s(dst));
}


// ir->rt should be OPE_INTEGER
void gen_asm_addi(Operand dst, Operand src, int imm)
{
    int first = ensure(src);
    int dest = allocate(dst);
    set_dirty(dest);
    if(first != dest)
        emit_asm(movl, "%s, %s", reg_to_s(first), reg_to_s(dest));
    emit_asm(addl, "$%d, %s", imm, reg_to_s(dest));
}


void gen_asm_add(IR *ir)
{
    if (ir->rt->type == OPE_INTEGER) {
        gen_asm_addi(ir->rd, ir->rs, ir->rt->integer);
    }
    else if (ir->rs->type == OPE_INTEGER) {
        gen_asm_addi(ir->rd, ir->rt, ir->rs->integer);
    }
    else {
        int first = ensure(ir->rs);
        int second = ensure(ir->rt);
        int dst = allocate(ir->rd);
        set_dirty(dst);
        if(dst == first){
            emit_asm(addl, "%s, %s", reg_to_s(second), reg_to_s(dst));
        }
        else if(dst == second){
            emit_asm(addl, "%s, %s", reg_to_s(first), reg_to_s(dst));
        } 
        else {
            emit_asm(movl, "%s, %s", reg_to_s(second), reg_to_s(dst));
            emit_asm(addl, "%s, %s", reg_to_s(first), reg_to_s(dst));
        }
    }
}


void gen_asm_sub(IR *ir)
{
    // Note, sub cannot exchange!
    if (ir->rt->type == OPE_INTEGER) {
        gen_asm_addi(ir->rd, ir->rs, -ir->rt->integer);
    }
    else {
        int first = ensure(ir->rs);
        int second = ensure(ir->rt);
        int dst = allocate(ir->rd);
        set_dirty(dst);
        if(dst == first){
            emit_asm(subl, "%s, %s", reg_to_s(second), reg_to_s(dst));
        }
        else if(dst == second){
            emit_asm(pushl, "%s", reg_to_s(first));
            emit_asm(subl, "%s, %s", reg_to_s(second), reg_to_s(first));
            emit_asm(movl, "%s, %s", reg_to_s(first), reg_to_s(dst));
            emit_asm(popl, "%s", reg_to_s(first));
        } 
        else {
            emit_asm(movl, "%s, %s", reg_to_s(first), reg_to_s(dst));
            emit_asm(subl, "%s, %s", reg_to_s(second), reg_to_s(dst));
        }
    }
}


void gen_asm_mul(IR *ir)
{
    int y = ensure(ir->rs);
    int z = ensure(ir->rt);
    int x = allocate(ir->rd);
    // Prepare for mul
    emit_asm(pushl, "%%eax");
    emit_asm(pushl, "%%ebx");
    emit_asm(pushl, "%%edx");
    if(y == EBX && z == EAX){
        emit_asm(movl, "%%eax, %%edx");
        emit_asm(movl, "%%ebx, %%eax");
        emit_asm(movl, "%%edx, %%ebx");
    } 
    else if(z != EAX){
        emit_asm(movl, "%s, %%EAX", reg_to_s(y));
        emit_asm(movl, "%s, %%EBX", reg_to_s(z));
    } else {
        emit_asm(movl, "%s, %%EBX", reg_to_s(z));
        emit_asm(movl, "%s, %%EAX", reg_to_s(y));
    }
    // EAX * EBX -> EDX:EAX
    set_dirty(x);
    emit_asm(mull, "%%ebx");
    // Restore registers.
    emit_asm(popl, "%%edx");
    emit_asm(popl, "%%ebx");
    if(x == EAX){
        emit_asm(addl, "$4, %%esp");
    } else {
        emit_asm(movl, "%%eax, %s", reg_to_s(x));
        emit_asm(popl, "%%eax");
    }
}


void gen_asm_div(IR *ir)
{
    int y = ensure(ir->rs);
    int z = ensure(ir->rt);
    int x = allocate(ir->rd);
    // Prepare for div
    emit_asm(pushl, "%%eax");
    emit_asm(pushl, "%%ebx");
    emit_asm(pushl, "%%edx");
    if(y == EBX && z == EAX){
        emit_asm(movl, "%%eax, %%edx");
        emit_asm(movl, "%%ebx, %%eax");
        emit_asm(movl, "%%edx, %%ebx");
    } 
    else if(z != EAX){
        emit_asm(movl, "%s, %%EAX", reg_to_s(y));
        emit_asm(movl, "%s, %%EBX", reg_to_s(z));
    } else {
        emit_asm(movl, "%s, %%EBX", reg_to_s(z));
        emit_asm(movl, "%s, %%EAX", reg_to_s(y));
    }
    emit_asm(movl, "$0, %%edx");
    // EDX:EAX / EBX -> EAX
    set_dirty(x);
    emit_asm(div, "%%ebx");
    // Restore registers.
    emit_asm(popl, "%%edx");
    emit_asm(popl, "%%ebx");
    if(x == EAX){
        emit_asm(addl, "$4, %%esp");
    } else {
        emit_asm(movl, "%%eax, %s", reg_to_s(x));
        emit_asm(popl, "%%eax");
    }
}


void gen_asm_load(IR *ir)
{
    int y = ensure(ir->rs);
    int x = allocate(ir->rd);
    set_dirty(x);
    emit_asm(movl, "0(%s), %s", reg_to_s(y), reg_to_s(x));
}


void gen_asm_store(IR *ir)
{
    int y = ensure(ir->rt);
    int x = ensure(ir->rs);
    emit_asm(movl, "%s, 0(%s)", reg_to_s(y), reg_to_s(x));
}


void gen_asm_goto(IR *ir)
{
    emit_asm(jmp, "%s", print_operand(ir->rs));
}


void gen_asm_arg(IR *ir)  // Not really emit code, but update the state.
{
    nr_arg++;
}


void gen_asm_call(IR *ir)
{
    // Open space for used save registers and arguments
    // Here we still manage stack by hand instead of the 'pushl' instruction to minimize the 
    // change of the original code.
    int offset = nr_arg * 4;
    if(offset != 0){
        emit_asm(addl, "$-%d, %%esp  # Open space for save and arguments", offset);
        sp_offset += offset;
    }

    push_all();

    IR *arg = ir;  // IR is stored consecutively


    // Push all arguments onto stack, which is more like x86 ;-)
    for (int i = 1; i <= nr_arg; i++) {

        do { arg--; } while (arg->type != IR_ARG);  // ARG may not be consecutive,
                                                    // so we use a iteration to find the first ARG
                                                    // before the current IR.
                                                    // It is expected that there always have enough
                                                    // ARG IRs that match [nr_arg]

        int y = ensure(arg->rs);
        emit_asm(movl, "%s, %d(%%esp)", reg_to_s(y), (i - 1) * 4);

    }

    emit_asm(call, "%s", ir->rs->name); // ?????

    clear_reg_state();

    int x = allocate(ir->rd);
    set_dirty(x);

    if ((ir->rd->next_use != MAX_LINE || ir->rd->liveness) && x != EAX) {
        emit_asm(movl, "%s, %%eax", reg_to_s(x));
    }

    if(offset != 0){
        emit_asm(addl, "$%d, %%esp  # Drawback save and arguments space", offset);
        sp_offset -= offset;
    }

    nr_arg = 0;  // After translating the call, clear arg state
}


//
// Update parameter's address
// Now we know whether the function has saved return address
// So we can fix the address of parameters.
//

void gen_asm_param(IR *ir)
{
    //if (curr_func->has_subroutine) {
        ir->rs->address -= 4;
    //}
}


void gen_asm_return(IR *ir)
{
    /* always use call instruction 
    if (curr_func->has_subroutine) {
        emit_asm(movl, "%d(%%esp), %%eax  # retrieve return address", sp_offset); //????
    }
    */
    
    int x = ensure(ir->rs);

    //int size = curr_func->has_subroutine ? curr_func->size + 4 : curr_func->size;
    int size = curr_func->size;
    if(size != 0)
        emit_asm(addl, "$%d, %%esp  # release stack space", size);
    if(x != EAX){
        emit_asm(movl, "%s, %%eax  # prepare return value", reg_to_s(x));
        emit_asm(ret, "");
    } else {
        emit_asm(ret, "           #return value prepared");
    }
}


void gen_asm_br(IR *ir)
{
    int x = ensure(ir->rs);
    int y = ensure(ir->rt);
    emit_asm(cmpl, "%s, %s", reg_to_s(x), reg_to_s(y));
    switch (ir->type) {
        case IR_BEQ: emit_asm(je, "%s", print_operand(ir->rd)); break;
        case IR_BNE: emit_asm(jne, "%s", print_operand(ir->rd)); break;
        case IR_BGT: emit_asm(jg, "%s",  print_operand(ir->rd)); break;
        case IR_BLT: emit_asm(jl, "%s",  print_operand(ir->rd)); break;
        case IR_BGE: emit_asm(jge, "%s", print_operand(ir->rd)); break;
        case IR_BLE: emit_asm(jle, "%s", print_operand(ir->rd)); break;
        default: assert(0);
    }
}


void gen_asm_dec(IR *ir)
{
    return;
}


void gen_asm_addr(IR *ir)
{
    int x = allocate(ir->rd);
    set_dirty(x);
    emit_asm(movl, "%%esp, %s  # get %s's address", reg_to_s(x), print_operand(ir->rs));
    emit_asm(addl, "$%d, %s  # get %s's address", sp_offset - ir->rs->address, reg_to_s(x), print_operand(ir->rs));
}


void gen_asm_write(IR *ir)
{
    int x = ensure(ir->rs);
    emit_asm(movl, "%s, %%eax", reg_to_s(x));
    emit_asm(call, "write");
}

void gen_asm_writec(IR *ir)
{
    int x = ensure(ir->rs);
    emit_asm(movl, "%s, %%eax", reg_to_s(x));
    emit_asm(call, "writec");
}


void gen_asm_read(IR *ir)
{
    int x = allocate(ir->rd);
    set_dirty(x);
    emit_asm(call, "read");
    emit_asm(movl, "%%eax, %s", reg_to_s(x));
}

void gen_asm_global(IR *ir)
{
    fprintf(asm_file, ".globl %s\n%s:\n", ir->rs->name, ir->rs->name);
    fprintf(asm_file, "  .long %d\n", ir->rt->integer);
}


typedef void(*trans_handler)(IR *);

trans_handler handler[NR_IR_TYPE] = {
    [IR_GLOBAL]  = gen_asm_global,
    [IR_DEC]     = gen_asm_dec,
    [IR_FUNC]    = gen_asm_func,
    [IR_ASSIGN]  = gen_asm_assign,
    [IR_ADD]     = gen_asm_add,
    [IR_SUB]     = gen_asm_sub,
    [IR_MUL]     = gen_asm_mul,
    [IR_DIV]     = gen_asm_div,
    [IR_LABEL]   = gen_asm_label,
    [IR_DEREF_R] = gen_asm_load,
    [IR_DEREF_L] = gen_asm_store,
    [IR_JMP]     = gen_asm_goto,
    [IR_ARG]     = gen_asm_arg,
    [IR_CALL]    = gen_asm_call,
    [IR_ADDR]    = gen_asm_addr,
    [IR_READ]    = gen_asm_read,
    [IR_WRITE]   = gen_asm_write,
    [IR_WRITEC]  = gen_asm_writec,
    [IR_PARAM]   = gen_asm_param,
    [IR_RET]     = gen_asm_return,
    [IR_BEQ]     = gen_asm_br,
    [IR_BLE]     = gen_asm_br,
    [IR_BGT]     = gen_asm_br,
    [IR_BGE]     = gen_asm_br,
    [IR_BNE]     = gen_asm_br,
    [IR_BLT]     = gen_asm_br
};

void gen_asm(IR *ir)
{
    fprintf(asm_file, "# %s\n", ir_to_s(ir));
    handler[ir->type](ir);
}

