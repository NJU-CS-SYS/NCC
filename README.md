# NCC

2016春季学期系统方向综合实验组成部分

## 目标

将类 C 的高级语言编译生成 x86 二进制可执行文件，由自制的内核加载运行。

## TODO

- [ ] 修改后端，生成 x86 二进制文件
- [ ] 完善前端，增加对 C 语言语法的支持

---

代码初始版本说明见下：

# Naive C-like Compiler

The code is derived from Wonicon's compiler lab code in school.
This NCC generates x86 binary files instead of MIP32 ones.
Since writing x86 back-end is confusing, we decide to use LLVM for the back-end.

## Supported Syntax

Some significant limitations are shown below:

1. No declaration, all functions should be defined before called;
1. No global variables (parsed but cannot generate code);
1. No preprocessor;
1. Basic type only consists of int (float is parsed but cannot generate code);
1. All local variables should be defined at the beginning of a function;
1. Struct is passed by reference, and returning a struct is dangerous.

The top level of a program should look like this:

```c
// one-line comment

/**
 * multi-line comment
 * can be /* nested */
 * ignore everything after //
 * like // */
 * like // /*
 * like // *//*
 * like // /*_*/
 */ // This is the end of this multi-line commet!

// int read()
// write(int x)
// are provided functions to support input and output

int echo(int e)
{
    write(e);
    return read();
}

struct A {
    int a;
    int b;
};

int main()
{
    /* Local definitions, like follows */

    int foo;
    int bar = 3;
    int vec[2];
    struct A a;

    /* Statements, like follows */

    if (/* Expression */) {
        /* Statements */
    }
    else if (/* Expression */) {
        /* Statements */
    }
    else {
        /* Statements */
    }

    while (/* Expression */) {
        /* Statements */
    }

    // the three parts of for-loop cannot be omitted currently
    for (foo = 0; foo < 10; foo = foo + 1) {
        write(foo);
    }

    /* Expressions */

    foo = bar;  // Assignment
    foo + 1;    // Binary operation: +, -, *, /
    foo < bar;  // Comparing operation: <, <=, >, >=, ==, !=
    !foo;       // Unary operation: -, !
    a.a         // field access
    (foo);
    foo = echo(vec[0]);  // Function call, array access

    return 0;
}

