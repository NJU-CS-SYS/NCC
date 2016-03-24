.data
.globl  _prompt
_prompt:
  .asciz "Enter an integer:"
.globl  _ret
_ret:
  .asciz "\n"
.text
.globl read
read:
  movl $66, %eax
  movl $_prompt, %ebx
  int $0x80
  ret
.globl write
write:
  movl %eax, %ebx
  movl $67, %eax
  movl $_ret, %ecx
  int $0x80
  syscall
  ret
.globl main

