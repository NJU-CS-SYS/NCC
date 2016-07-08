# NCC 文法
本文档中将给出 NCC 所编译的简易 C 语言的文法定义和补充说明。本文法基于南京大学编译原理课程实验所用的 C-- 文法，所以文档的大部分内容与许畅教授编译原理实验讲义中的 *《附录A: C--语言文法》* 内容相同。

## 文法定义
### Tokens
INT -> /\* A sequence of digits without spaces[^1] \*/  
FLOAT -> /\* A real number consisting of digits and one decimal point. The decimal point must be surrounded by at least one digits[^2] \*/   
CHAR -> /\* A character enclosed with '' \*/  
ID -> /\* A character string consisting of 52 upper- or lower-case alphabetic, 10 numeric and one underscore characters. Besides, an identifier must not start with a digit */ 
SEMI -> ;  
COMMA -> ,  
ASSIGNOP -> =  
RELOP -> > | < | >= | <= | == | !=   
PLUS -> +  
MINUS -> -  
STAR -> *  
DIV -> /  
AND -> &&  
OR -> ||  
DOT -> .   
NOT -> !   
TYPE ->  
LP -> (  
RP -> )  
LB -> [  
RB -> ]  
LC -> {  
RC -> }  
STRUCT -> struct   
RETURN -> return   
IF -> if  
ELSE -> else   
WHILE -> while

[^1]: This integer can be either in decimal format, octal format or hexadecimal format. Actually, this integer is treated as unsigned interger in C language, because there is no negative integer under this syntax.  
[^2]: This float must has a number on both sides of the '.'. And the scientific notation is not supported. Futher more, the float can be parsed but cannot be used to generate codes.

### High-level Definitions
Program -> ExtDefList  
ExtDefList -> ExtDef ExtDefList   
	| ε  
ExtDef -> Specifier ExtDecList SEMI  
    | Specifier SEMI  
	| Specifier FunDec CompSt   
ExtDecList -> ExtDec  
    | ExtDec COMMA ExtDecList
ExtDec -> VarDec
    | VarDec ASSIGNOP INT
    
### Specifiers
Specifier -> TYPE  
    | StructSpecifier  
StructSpecifier -> STRUCT ID LC DefList RC  
	| STRUCT LC DefList RC   
    | STRUCT ID   

### Declarators
VarDec -> ID  
    | VarDec LB INT RB  
FunDec -> ID LP VarList RP   
	| ID LP RP  
VarList -> ParamDec COMMA VarList   
	| ParamDec  
ParamDec -> Specifier VarDec  

### Statements
CompSt -> LC DefList StmtList RC   
StmtList -> Stmt StmtList  
    | ε  
Stmt -> Exp SEMI  
    | CompSt  
    | RETURN Exp SEMI  
    | IF LP Exp RP Stmt SUB_ELSE  
    | IF LP Exp RP Stmt ELSE Stmt  
    | WHILE LP Exp RP Stmt  
    | FOR LP Exp SEMI Exp SEMI exp RP Stmt  

### Local Definitions
DefList -> Def DefList   
	| ε  
Def -> Specifier DecList SEMI   
DecList -> Dec  
    | Dec COMMA DecList   
Dec -> VarDec  
    | VarDec ASSIGNOP Exp  
   
### Expressions
Exp -> Exp ASSIGNOP Exp  
	| Exp AND Exp  
    | Exp OR Exp  
    | Exp RELOP Exp  
	| Exp PLUS Exp  
	| Exp MINUS Exp  
	| Exp STAR Exp  
	| Exp DIV Exp  
    | LP Exp RP  
    | MINUS Exp  
    | NOT Exp  
    | ID LP Args RP  
    | ID LP RP  
    | Exp LB Exp RB  
    | Exp DOT ID  
    | ID  
    | INT  
    | FLOAT  
 	| CHAR  
Args -> Exp COMMA Args   
	| Exp  

## 文法说明
### 优先级与结合性
 优先级	| 		运算符 			| 结合性 | 描述 
------	|	------------		|-------|-----
	1 	| LP RP LB RB DOT 	| 左结合 | ( ) [ ] . 
	2 	| NOT 					| 右结合 | ! 
 	3	| STAR DIV 			| 左结合 | * / 
 	4 	| PLUS MINUS 			| 左结合 | + - 
 	5 	| RELOP 				| 左结合 | > < >= <= == != 
 	6 	| AND	 				| 左结合 | && 
 	7 	| OR 					| 左结合 | \|\| 
 	8 	| ASSIGNOP 			| 右结合 | = 

### 注释
NCC 支持 C 语言风格的 /* \*/ 和 // 注释，即忽略 // 后的本行内容和 /* 与 \*/了两个符号间的所有内容，并在此基础之上增加了注释嵌套的支持，但 /* 与 */ 必须成对。**注意：**若出现单行注释符号 //，则 // 后面的注释符号也会被忽略掉。

### 系统调用
为了在 NEMU 上完成输入输出，增加了两个系统调用，分别为 `int read()` 和 `int write(int)`，均用 `int $0x80` 进入中断。

`int read()` 的系统调用编号为 66，用于从系统中获取输入的整型数，并将该整型数作为返回值返回。

`int write(int)` 的系统调用编号为 67，用于向屏幕打印整型数，要打印整型数作为参数传入，返回值无意义。

`int writec(char)` 的系统调用编号为 68，用于向屏幕打印字符，要打印的字符作为参数传入，返回值无意义

### 字符数组
在 NCC 中字符的内存空间占用大小，储存方式均与整形数一致，赋值也需要单个字符进行赋值，暂时不支持字符串赋值，这种储存方式避免了对齐的问题，同时整形数和字符的一致性使得字符可以和整形数混合进行运算，方便一些情况下的输出。
