.section __data,__DATA
.globl  _prompt
_prompt:
  .asciz "Enter an integer:"
.globl  _ret
_ret:
  .asciz "\n"
.section __text,__TEXT
.globl _read
_read:
  movl $66, %eax
  movl $_prompt, %ebx
  int $0x80
  ret
.globl _write
_write:
  movl %eax, %ebx
  movl $67, %eax
  movl $_ret, %ecx
  int $0x80
  syscall
  ret
.globl _main

