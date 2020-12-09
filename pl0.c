/*
 * PL/0 complier program for win32 platform (implemented in C)
 *
 * The program has been test on Visual C++ 6.0, Visual C++.NET and
 * Visual C++.NET 2003, on Win98, WinNT, Win2000, WinXP and Win2003
 *
 * 使用方法：
 * 运行后输入PL/0源程序文件?
 * 回答是否输出虚拟机代码
 * 回答是否输出名字表
 * fa.tmp输出虚拟机代码
 * fa1.tmp输出源文件及其各行对应的首地址
 * fa2.tmp输出结?
 * fas.tmp输出名字表
 */

#include <stdio.h>

#include "pl0.h"
#include "string.h"

/* 解释执行时使用的栈 */
#define stacksize 500


int main()
{
    bool nxtlev[symnum];

    // printf("Input pl/0 file?   ");
    // scanf("%s", fname);     /* 输入文件名 */

    fin = fopen("C:\\Users\\Tony\\CLionProjects\\CompilationPrinciple\\Examples\\TempTest2.pl0", "r");
    // fin = fopen("/Users/tony/实验室/ClionProjects/PL0-Experiment/Examples/function.pl0", "r");

    if (fin)
    {
        printf("List object virtualCode?(Y/N)");   /* 是否输出虚拟机代码 */
        scanf("%s", fname);
        listswitch = (fname[0]=='y' || fname[0]=='Y');

        printf("List symbol nameTable?(Y/N)");  /* 是否输出名字表 */
        scanf("%s", fname);
        tableswitch = (fname[0]=='y' || fname[0]=='Y');

        fa1 = fopen("fa1.tmp", "w");
        fprintf(fa1,"Input pl/0 file?   ");
        fprintf(fa1,"%s\n",fname);

        init();     /* 初始化 */

        err = 0;
        currentCharIndex = virtualMachineCodePointer = newLineIndex = 0;
        ch = ' ';

        if(-1 != getsym())
        {
            fa = fopen("fa.tmp", "w");
            fas = fopen("fas.tmp", "w");
            addset(nxtlev, declbegsys, statbegsys, symnum);
            nxtlev[period] = true; // nxtlev中含有声明开始符和语句开始符

            if(-1 == block(0, 0, nxtlev))   /* 调用编译程序 */
            {
                fclose(fa);
                fclose(fa1);
                fclose(fas);
                fclose(fin);
                printf("\n");
                return 0;
            }
            fclose(fa);
            fclose(fa1);
            fclose(fas);

            if (sym != period)
            {
                error(9);
            }

            if (err == 0)
            {
                fa2 = fopen("fa2.tmp", "w");
                interpret();    /* 调用解释执行程序 */
                fclose(fa2);
            }
            else
            {
                printf("Errors in pl/0 program");
            }
        }

        fclose(fin);
    }
    else
    {
        printf("Can't open file!\n");
    }

    printf("\n");
    return 0;
}

/*
* 初始化
*/
void init()
{
    int i;

    /* 设置单字符符号 */
    for (i=0; i<=255; i++)
    {
        ssym[i] = nul;
    }
    ssym['+'] = plus;
    ssym['-'] = minus;
    ssym['*'] = times;
    ssym['/'] = slash;
    ssym['('] = lparen;
    ssym[')'] = rparen;
    ssym['='] = eql;
    ssym[','] = comma;
    ssym['.'] = period;
    ssym['#'] = neq;
    ssym[';'] = semicolon;

    /* 设置保留字名字,按照字母顺序，便于折半查找 */
    strcpy(&(word[0][0]), "begin");
    strcpy(&(word[1][0]), "call");
    strcpy(&(word[2][0]), "const");
    strcpy(&(word[3][0]), "do");
    strcpy(&(word[4][0]), "else");
    strcpy(&(word[5][0]), "end");
    strcpy(&(word[6][0]), "if");
    strcpy(&(word[7][0]), "odd");
    strcpy(&(word[8][0]), "procedure");
    strcpy(&(word[9][0]), "read");
    strcpy(&(word[10][0]), "then");
    strcpy(&(word[11][0]), "var");
    strcpy(&(word[12][0]), "while");
    strcpy(&(word[13][0]), "write");

    /* 设置保留字符号 */
    wsym[0] = beginsym;
    wsym[1] = callsym;
    wsym[2] = constsym;
    wsym[3] = dosym;
    wsym[4] = elsesym;
    wsym[5] = endsym;
    wsym[6] = ifsym;
    wsym[7] = oddsym;
    wsym[8] = procsym;
    wsym[9] = readsym;
    wsym[10] = thensym;
    wsym[11] = varsym;
    wsym[12] = whilesym;
    wsym[13] = writesym;

    /* 设置指令名称 */
    strcpy(&(mnemonic[lit][0]), "lit");
    strcpy(&(mnemonic[opr][0]), "opr");
    strcpy(&(mnemonic[lod][0]), "lod");
    strcpy(&(mnemonic[sto][0]), "sto");
    strcpy(&(mnemonic[cal][0]), "cal");
    strcpy(&(mnemonic[inte][0]), "int");
    strcpy(&(mnemonic[jmp][0]), "jmp");
    strcpy(&(mnemonic[jpc][0]), "jpc");
    strcpy(&(mnemonic[lda][0]), "lda");
    strcpy(&(mnemonic[sta][0]), "sta");

    /* 设置符号集 */
    for (i=0; i<symnum; i++)
    {
        declbegsys[i] = false;
        statbegsys[i] = false;
        facbegsys[i] = false;
    }

    /* 设置声明开始符号集 */
    declbegsys[constsym] = true;
    declbegsys[varsym] = true;
    declbegsys[procsym] = true;

    /* 设置语句开始符号集 */
    statbegsys[beginsym] = true;
    statbegsys[callsym] = true;
    statbegsys[ifsym] = true;
    statbegsys[whilesym] = true;
    statbegsys[readsym] = true;			// thanks to elu
    statbegsys[writesym] = true;

    /* 设置因子开始符号集 */
    facbegsys[ident] = true;
    facbegsys[number] = true;
    facbegsys[lparen] = true;

}

/*
* 用数组实现集合的集合运算
*/
int inset(int e, bool* s)
{
    return s[e];
}

int addset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i=0; i<n; i++)
    {
        sr[i] = s1[i]||s2[i];
    }
    return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i=0; i<n; i++)
    {
        sr[i] = s1[i]&&(!s2[i]);
    }
    return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i=0; i<n; i++)
    {
        sr[i] = s1[i]&&s2[i];
    }
    return 0;
}

/*
*   出错处理，打印出错位置和错误编码
*/
void error(int n)
{
    char space[81];
    memset(space,32,81);

    space[currentCharIndex - 1]=0; //出错时当前符号已经读完，所以cc-1

    printf("****%s!%d\n", space, n);
    fprintf(fa1,"****%s!%d\n", space, n);

    err++;
}

/*
* 漏掉空格，读取一个字符。
*
* 每次读一行，存入line缓冲区，line被getsym取空后再读一行
*
* 被函数getsym调用。
*/
int getch()
{
    if (currentCharIndex == newLineIndex)
    {
        if (feof(fin)) // 检测是否遇到文件结束符
        {
            printf("program incomplete");
            return -1;
        }

        newLineIndex=0;
        currentCharIndex=0;
        printf("%d ", virtualMachineCodePointer);
        fprintf(fa1, "%d ", virtualMachineCodePointer);
        ch = ' ';
        while (ch != 10) // 判断字符缓冲区中的字符是否是换行符
        {
            //fscanf(fin,"%c", &ch)
            //richard
            if (EOF == fscanf(fin,"%c", &ch))
            {
                line[newLineIndex] = 0;
                break;
            }
            //end richard
            printf("%c", ch);
            fprintf(fa1, "%c", ch);
            line[newLineIndex] = ch;
            newLineIndex++;
        }
        printf("\n");
        fprintf(fa1, "\n");
    }
    ch = line[currentCharIndex];
    currentCharIndex++;
    return 0;
}

/*
* 词法分析，获取一个符号
*/
int getsym()
{
    int i,j,k;

    /* the original version lacks "\r", thanks to foolevery */
    while (ch==' ' || ch==10 || ch==13 || ch==9)  /* 忽略空格、换行、回车和TAB */
    {
        getchdo;
        //在本循环中第一次执行完毕后line中为源pl0文件中的某一行代码（即字符串），ch为本行第一个字符，
        //cc指向ch所在下标+1，
    }
    if (ch>='a' && ch<='z')
    {           /* 名字或保留字以a..z开头 */
        k = 0;
        do {
            if(k<al)
            {
                a[k] = ch;
                k++;
            }
            getchdo;
        } while (ch>='a' && ch<='z' || ch>='0' && ch<='9');
        a[k] = 0;
        strcpy(id, a);// id为a~z开头、只包含a、z的字符串，用于判断是标识符还是保留字
        i = 0;
        j = norw-1;
        do {    /* 使用二分查找来搜索当前符号是否为保留字 */
            k = (i+j)/2;
            if (strcmp(id,word[k]) <= 0)
            {
                j = k - 1;
            }
            if (strcmp(id,word[k]) >= 0)
            {
                i = k + 1;
            }
        } while (i <= j);
        if (i-1 > j)
        {
            sym = wsym[k];
        }
        else
        {
            sym = ident; /* 搜索失败则，是名字或数字 */
        }
    }
    else
    {
        if (ch>='0' && ch<='9')
        {           /* 检测是否为数字：以0..9开头 */
            k = 0;
            num = 0;
            sym = number;
            do {
                num = 10*num + ch - '0';
                k++;
                getchdo;
            } while (ch>='0' && ch<='9'); /* 获取数字的值 */
            k--;
            if (k > nmax)
            {
                error(30);
            }
        }
        else
        {
            if (ch == ':')      /* 检测赋值符号 */
            {
                getchdo;
                if (ch == '=')
                {
                    sym = becomes;
                    getchdo;
                }
                else
                {
                    sym = colon; //加入数组后可识别成冒号
                    // sym = nul;  /* 不能识别的符号 */
                }
            }
            else
            {
                if (ch == '<')      /* 检测小于或小于等于符号 */
                {
                    getchdo;
                    if (ch == '=')
                    {
                        sym = leq;
                        getchdo;
                    }
                    else
                    {
                        sym = lss;
                    }
                }
                else
                {
                    if (ch=='>')        /* 检测大于或大于等于符号 */
                    {
                        getchdo;
                        if (ch == '=')
                        {
                            sym = geq;
                            getchdo;
                        }
                        else
                        {
                            sym = gtr;
                        }
                    }
                    else
                    {

                        if(ch == '{') {
//                            do {
//                                if(currentCharIndex == newLineIndex && ch != '}') {
//                                    error(31); //没有匹配的右括号
//                                }
//                                getchdo;
//                            } while(ch != '}');
//                            getchdo;
//                            getsymdo; //获取下一个符号，下一个符号为真实符号，当前'}'符号没有意义
                            while(ch != '}') {
                                getchdo;
                            }
                            getchdo;
                            getsymdo;
                        } else {
                            sym = ssym[ch];     /* 当符号不满足上述条件时，全部按照单字符符号处理 */
                            //getchdo;
                            //richard
                            if (sym != period)
                            {
                                getchdo;
                            }
                            //end richard
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/*
* 生成虚拟机代码
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
int gen(enum fct x, int y, int z )
{
    if (virtualMachineCodePointer >= maxVirtualMachineCodeNum)
    {
        printf("Program too long"); /* 程序过长 */
        return -1;
    }
    virtualCode[virtualMachineCodePointer].f = x;
    virtualCode[virtualMachineCodePointer].l = y;
    virtualCode[virtualMachineCodePointer].a = z;
    virtualMachineCodePointer++;
    return 0;
}


/*
* 测试当前符号是否合法
*
* 在某一部分（如一条语句，一个表达式）将要结束时时我们希望下一个符号属于某集?
* （该部分的后跟符号），test负责这项检测，并且负责当检测不通过时的补救措施，
* 程序在需要检测时指定当前需要的符号集合和补救用的集合（如之前未完成部分的后跟
* 符号），以及检测不通过时的错误号。
*
* s1:   我们需要的符号
* s2:   如果不是我们需要的，则需要一个补救用的集?
* n:    错误号
*/
int test(bool* s1, bool* s2, int n)
{
    if (!inset(sym, s1))
    {
        error(n);
        /* 当检测不通过时，不停获取符号，直到它属于需要的集合或补救的集合 */
        while ((!inset(sym,s1)) && (!inset(sym,s2)))
        {
            getsymdo;
        }
    }
    return 0;
}

/*
* 编译程序主?
*
* eachProgramLevel:    当前分程序所在层
* nameTableTailPointer:     名字表当前尾指针
* fsys:   当前模块后跟符号集?
*/
int block(int eachProgramLevel, int nameTableTailPointer, bool* fsys)// 分程序处理过程
{
    int i;

    int nameRelativeAddress;                 /* 名字分配到的相对地址 */
    int nameTableTailPointer0;                /* 保留初始tx */
    int virtualMachineCodePointer0;                /* 保留初始cx */
    bool nxtlev[symnum];    /* 在下级函数的参数中，符号集合均为值参，但由于使用数组实现，
							传递进来的是指针，为防止下级函数改变上级函数的集合，开辟新的空?
							传递给下级函数*/

    nameRelativeAddress = 3;
    nameTableTailPointer0 = nameTableTailPointer;               /* 记录本层名字的初始位置 */
    nameTable[nameTableTailPointer].adr = virtualMachineCodePointer;

    gendo(jmp, 0, 0);

    if (eachProgramLevel > maxNestLevelNum)
    {
        error(32);
    }

    do {

        if (sym == constsym)    /* 收到常量声明符号，开始处理常量声明 */
        {
            getsymdo;

            /* the original do...while(sym == ident) is problematic, thanks to calculous */
            /* do { */
            constdeclarationdo(&nameTableTailPointer, eachProgramLevel, &nameRelativeAddress);  /* dx的值会被constdeclaration改变，使用指针 */
            while (sym == comma) // 逗号，表示不止声明一个常量，需要循环调用getsymdo函数继续寻找常量
            {
                getsymdo;
                constdeclarationdo(&nameTableTailPointer, eachProgramLevel, &nameRelativeAddress);
            }
            if (sym == semicolon)
            {
                getsymdo;
            }
            else
            {
                error(5);   /*漏掉了逗号或者分号*/
            }
            /* } while (sym == ident); */
        }

        if (sym == varsym)      /* 收到变量声明符号，开始处理变量声明 */
        {
            getsymdo;

            vardeclarationdo(&nameTableTailPointer, eachProgramLevel, &nameRelativeAddress);
            while (sym == comma) // 逗号，表示不止声明一个变量，需要循环调用getsymdo函数继续寻找变量
            {
                getsymdo;
                vardeclarationdo(&nameTableTailPointer, eachProgramLevel, &nameRelativeAddress);
            }
            if (sym == semicolon)
            {
                getsymdo;
            }
            else
            {
                error(5);
            }
            /* } while (sym == ident);  */
        }

        while (sym == procsym) /* 收到过程声明符号，开始处理过程声明 */
        {
            getsymdo;

            if (sym == ident)
            {
                enter(procedur, &nameTableTailPointer, eachProgramLevel, &nameRelativeAddress); /* 记录过程名字 */
                getsymdo;
            }
            else
            {
                error(4);   /* procedure后应为标识符 */
            }

            if (sym == semicolon)
            {
                getsymdo;
            }
            else
            {
                error(5);   /* 漏掉了分号 */
            }

            memcpy(nxtlev, fsys, sizeof(bool)*symnum);
            nxtlev[semicolon] = true;
            if (-1 == block(eachProgramLevel + 1, nameTableTailPointer, nxtlev))
            {
                return -1;  /* 递归调用 */
            }

            if(sym == semicolon)
            {
                getsymdo;
                memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
                nxtlev[ident] = true;
                nxtlev[procsym] = true;
                testdo(nxtlev, fsys, 6);
            }
            else
            {
                error(5);   /* 漏掉了分号 */
            }
        }
        memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
        nxtlev[ident] = true;
        testdo(nxtlev, declbegsys, 7);
    } while (inset(sym, declbegsys));   /* 直到没有声明符号 */

    virtualCode[nameTable[nameTableTailPointer0].adr].a = virtualMachineCodePointer;    /* 开始生成当前过程代码 */
    nameTable[nameTableTailPointer0].adr = virtualMachineCodePointer;            /* 当前过程代码地址 */
    nameTable[nameTableTailPointer0].size = nameRelativeAddress;           /* 声明部分中每增加一条声明都会给dx增加1，声明部分已经结束，dx就是当前过程数据的size */
    virtualMachineCodePointer0 = virtualMachineCodePointer;
    gendo(inte, 0, nameRelativeAddress);             /* 生成分配内存代码 */

    if (tableswitch)        /* 输出名字表 */
    {
        printf("TABLE:\n");
        if (nameTableTailPointer0 + 1 > nameTableTailPointer)
        {
            printf("    NULL\n");
        }
        for (i= nameTableTailPointer0 + 1; i <= nameTableTailPointer; i++)
        {
            switch (nameTable[i].kind)
            {
                case constant:
                    printf("    %d const %s ", i, nameTable[i].name);
                    printf("val=%d\n", nameTable[i].val);
                    fprintf(fas, "    %d const %s ", i, nameTable[i].name);
                    fprintf(fas, "val=%d\n", nameTable[i].val);
                    break;
                case variable:
                    printf("    %d var   %s ", i, nameTable[i].name);
                    printf("eachProgramLevel=%d addr=%d\n", nameTable[i].level, nameTable[i].adr);
                    fprintf(fas, "    %d var   %s ", i, nameTable[i].name);
                    fprintf(fas, "eachProgramLevel=%d addr=%d\n", nameTable[i].level, nameTable[i].adr);
                    break;
                case procedur:
                    printf("    %d proc  %s ", i, nameTable[i].name);
                    printf("eachProgramLevel=%d addr=%d size=%d\n", nameTable[i].level, nameTable[i].adr, nameTable[i].size);
                    fprintf(fas, "    %d proc  %s ", i, nameTable[i].name);
                    fprintf(fas, "eachProgramLevel=%d addr=%d size=%d\n", nameTable[i].level, nameTable[i].adr, nameTable[i].size);
                    break;
                case arrays:
                    printf("    %d array %s ", i, nameTable[i].name);
                    printf("eachProgramLevel=%d addr=%d size=%d\n", nameTable[i].level, nameTable[i].adr, nameTable[i].size);
                    fprintf(fas, "    %d array  %s ", i, nameTable[i].name);
                    fprintf(fas, "eachProgramLevel=%d addr=%d size=%d\n", nameTable[i].level, nameTable[i].adr, nameTable[i].size);
            }
        }
        printf("\n");
    }

    /* 语句后跟符号为分号或end */
    memcpy(nxtlev, fsys, sizeof(bool)*symnum);  /* 每个后跟符号集和都包含上层后跟符号集和，以便补救 */
    nxtlev[semicolon] = true;
    nxtlev[endsym] = true;
    statementdo(nxtlev, &nameTableTailPointer, eachProgramLevel);
    gendo(opr, 0, 0);                       /* 每个过程出口都要使用的释放数据段指令 */
    memset(nxtlev, 0, sizeof(bool)*symnum); /*分程序没有补救集合 */
    testdo(fsys, nxtlev, 8);                /* 检测后跟符号正确性 */
    listcode(virtualMachineCodePointer0);                          /* 输出代码 */
    return 0;
}

/*
* 在名字表中加入一项
*
* k:      名字种类const,var or procedure
* ptx:    名字表尾指针的指针，为了可以改变名字表尾指针的值
* lev:    名字所在的层次,，以后所有的lev都是这样
* pdx:    dx为当前应分配的变量的相对地址，分配后要增加1
*/
void enter(enum object k, int* ptx, int lev, int* pdx)
{
    (*ptx)++;
    strcpy(nameTable[(*ptx)].name, id); /* 全局变量id中已存有当前名字的名字 */
    nameTable[(*ptx)].kind = k;
    switch (k)
    {
        case constant:  /* 常量名字 */
            if (num > amax)
            {
                error(31);  /* 数越界 */
                num = 0;
            }
            nameTable[(*ptx)].val = num;
            break;
        case variable:  /* 变量名字 */
            nameTable[(*ptx)].level = lev;
            nameTable[(*ptx)].adr = (*pdx);
            (*pdx)++;
            break;
        case procedur:  /*　过程名字　*/
            nameTable[(*ptx)].level = lev;
            break;
        case arrays: // 数组名,进行记录下界等
            nameTable[(*ptx)].level = lev;
            nameTable[(*ptx)].adr = (*pdx);
            nameTable[(*ptx)].data = g_arrBase;
            nameTable[(*ptx)].size = g_arrSize;
            *pdx = (*pdx) + g_arrSize;
            break;
    }
}

/*
* 查找名字的位置.
* 找到则返回在名字表中的位置,否则返回0.
*
* idt:    要查找的名字
* tx:     当前名字表尾指针
*/
int position(char* idt, int tx)
{
    int i;
    strcpy(nameTable[0].name, idt);
    i = tx;
    while (strcmp(nameTable[i].name, idt) != 0)
    {
        i--;
    }
    return i;
}

/*
* 常量声明处理
*/
int constdeclaration(int* nameTableTailPointer, int eachProgramLevel, int* nameRelativeAddress)
{
    if (sym == ident)
    {
        getsymdo;
        if (sym==eql || sym==becomes)
        {
            if (sym == becomes)
            {
                error(1);   /* 把=写成了:= */
            }
            getsymdo;
            if (sym == number)
            {
                enter(constant, nameTableTailPointer, eachProgramLevel, nameRelativeAddress);
                getsymdo;
            }
            else
            {
                error(2);   /* 常量说明=后应是数字 */
            }
        }
        else
        {
            error(3);   /* 常量说明标识后应是= */
        }
    }
    else
    {
        error(4);   /* const后应是标识 */
    }
    return 0;
}

/*
* 变量声明处理
*/
int vardeclaration(int* ptx,int lev,int* pdx)
{
    if (sym == ident)
    {
        int isArray = arraydeclaration(ptx, lev, pdx);
        if(isArray == 1) {
            enter(arrays, ptx, lev, pdx);
            getsymdo;
        } else if(isArray == 0) {
            enter(variable, ptx, lev, pdx);
        } else {
            return -1;
        }
        // getsymdo;
    }
    else
    {
        error(4);   /* var后应是标识 */
    }
    return 0;
}

/*
* 输出目标代码清单
*/
void listcode(int cx0)
{
    int i;
    if (listswitch)
    {
        for (i=cx0; i < virtualMachineCodePointer; i++)
        {
            printf("%d %s %d %d\n", i, mnemonic[virtualCode[i].f], virtualCode[i].l, virtualCode[i].a);
            fprintf(fa, "%d %s %d %d\n", i, mnemonic[virtualCode[i].f], virtualCode[i].l, virtualCode[i].a);
        }
    }
}

/*
* 语句处理
*/
int statement(bool* fsys, int* ptx, int lev)
{
    int i, virtualMachineCodePointer1, virtualMachineCodePointer2, virtualMachineCodePointer3;
    bool nxtlev[symnum];

    if (sym == ident)   /* 准备按照赋值语句处理 */
    {
        i = position(id, *ptx);
        if (i == 0)
        {
            error(11);  /* 变量未找到 */
        }
        else
        {
            if(nameTable[i].kind != variable && nameTable[i].kind != arrays)
            {
                error(12);  /* 赋值语句格式错误 */
                i = 0;
            }
            else
            {
                enum fct fct1;
                if(nameTable[i].kind == arrays) {
                    arraycoefdo(fsys, ptx, lev);
                    fct1 = sta;  /* 数组保存,要多读一个栈*/
                } else {
                    fct1 = sto;
                }

                getsymdo;
                if(sym == becomes)
                {
                    getsymdo;
                }
                else
                {
                    error(13);  /* 没有检测到赋值符号 */
                }
                memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                expressiondo(nxtlev, ptx, lev); /* 处理赋值符号右侧表达式 */
                if(i != 0)
                {
                    // 对于变量，expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值
                    // 对于数组，同上
                    gendo(fct1, lev - nameTable[i].level, nameTable[i].adr);
                }
            }
        }//if (i == 0)
    }
    else
    {
        if (sym == readsym) /* 准备按照read语句处理 */
        {
            getsymdo;
            if (sym != lparen)
            {
                error(34);  /* 格式错误，应是左括号 */
            }
            else
            {
                do {
                    getsymdo;
                    if (sym == ident)
                    {
                        i = position(id, *ptx); /* 查找要读的变量 */
                    }
                    else
                    {
                        i=0;
                    }

                    if (i == 0)
                    {
                        error(35);  /* read()中应是声明过的变量名 */
                    }
                    else if (nameTable[i].kind != variable && nameTable[i].kind != arrays)
                    {
                        error(32);	/* read()参数表的标识符不是变量, thanks to amd */
                    }
                    else
                    {
                        enum fct fct1;
                        if(nameTable[i].kind == arrays) {
                            arraycoefdo(fsys, ptx, lev); //执行完此处指令序列后，栈顶值为数组空间的偏移地址
                            fct1 = sta;
                        } else {
                            fct1 = sto;
                        }
                        gendo(opr, 0, 16);  // 生成输入指令，读取值到栈顶
                        gendo(fct1, lev - nameTable[i].level, nameTable[i].adr); //此时栈顶值为输入数值，次栈顶为数组空间的偏移地址，栈顶值存入（数组基址（即数组在字母表中存储的adr）+偏移地址）
                    }
                    getsymdo;

                } while (sym == comma); /* 一条read语句可读多个变量 */
            }
            if(sym != rparen)
            {
                error(33);  /* 格式错误，应是右括号 */
                while (!inset(sym, fsys))   /* 出错补救，直到收到上层函数的后跟符号 */
                {
                    getsymdo;
                }
            }
            else
            {
                getsymdo;
            }
        }
        else
        {
            if (sym == writesym)    /* 准备按照write语句处理，与read类似 */
            {
                getsymdo;
                if (sym == lparen)
                {
                    do {
                        getsymdo;
                        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                        nxtlev[rparen] = true;
                        nxtlev[comma] = true;       /* write的后跟符号为) or , */
                        expressiondo(nxtlev, ptx, lev); /* 调用表达式处理，此处与read不同，read为给变量赋值 */
                        gendo(opr, 0, 14);  /* 生成输出指令，输出栈顶的值 */
                    } while (sym == comma);
                    if (sym != rparen)
                    {
                        error(33);  /* write()中应为完整表达式 */
                    }
                    else
                    {
                        getsymdo;
                    }
                }
                gendo(opr, 0, 15);  /* 输出换行 */
            }
            else
            {
                if (sym == callsym) /* 准备按照call语句处理 */
                {
                    getsymdo;
                    if (sym != ident)
                    {
                        error(14);  /* call后应为标识符 */
                    }
                    else
                    {
                        i = position(id, *ptx);
                        if (i == 0)
                        {
                            error(11);  /* 过程未找到 */
                        }
                        else
                        {
                            if (nameTable[i].kind == procedur)
                            {
                                gendo(cal, lev - nameTable[i].level, nameTable[i].adr);   /* 生成call指令 */
                            }
                            else
                            {
                                error(15);  /* call后标识符应为过程 */
                            }
                        }
                        getsymdo;
                    }
                }
                else
                {
                    if (sym == ifsym)   /* 准备按照if语句处理 */
                    {
                        getsymdo;
                        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                        nxtlev[thensym] = true;
                        nxtlev[dosym] = true;   /* 后跟符号为then或do */
                        conditiondo(nxtlev, ptx, lev); /* 调用条件处理（逻辑运算）函数 */
                        if (sym == thensym)
                        {
                            getsymdo;
                        }
                        else
                        {
                            error(16);  /* 缺少then */
                        }
                        virtualMachineCodePointer1 = virtualMachineCodePointer;   /* 保存当前指令地址 */
                        gendo(jpc, 0, 0);   /* 生成条件跳转指令，跳转地址未知，暂时写0 */
                        statementdo(fsys, ptx, lev);    /* 处理then后的语句 */

                        if(sym == semicolon) {
                            getsymdo;
                            if(sym == elsesym) {
                                getsymdo;
                                virtualMachineCodePointer2 = virtualMachineCodePointer;
                                /*cx为当前的指令地址，cx+1即为then语句执行后的else语句的位置，回填地址*/
                                virtualCode[virtualMachineCodePointer1].a = virtualMachineCodePointer + 1;
                                gendo(jmp, 0, 0);
                                statementdo(fsys, ptx, lev);
                                /*经statement处理后，cx为else后语句执行
                                完的位置，它正是前面未定的跳转地址，回填地址*/
                                virtualCode[virtualMachineCodePointer2].a = virtualMachineCodePointer;
                            } else {
                                /*经statement处理后，cx为then后语句执行完的位置，它正是前面未定的跳转地址*/
                                virtualCode[virtualMachineCodePointer1].a = virtualMachineCodePointer;
                            }
                        } else {
                            error(5);
                        }
                    }
                    else
                    {
                        if (sym == beginsym)    /* 准备按照复合语句处理 */
                        {
                            getsymdo;
                            memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                            nxtlev[semicolon] = true;
                            nxtlev[endsym] = true;  /* 后跟符号为分号或end */
                            /* 循环调用语句处理函数，直到下一个符号不是语句开始符号或收到end */
                            statementdo(nxtlev, ptx, lev);

                            while (inset(sym, statbegsys) || sym==semicolon)
                            {
                                if (sym == semicolon)
                                {
                                    getsymdo;
                                }
                                else
                                {
                                    error(10);  /* 缺少分号 */
                                }
                                statementdo(nxtlev, ptx, lev);
                            }
                            if(sym == endsym)
                            {
                                getsymdo;
                            }
                            else
                            {
                                error(17);  /* 缺少end或分号 */
                            }
                        }
                        else
                        {
                            if (sym == whilesym)    /* 准备按照while语句处理 */
                            {
                                virtualMachineCodePointer1 = virtualMachineCodePointer;   /* 保存判断条件操作的位置 */
                                getsymdo;
                                memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                                nxtlev[dosym] = true;   /* 后跟符号为do */
                                conditiondo(nxtlev, ptx, lev);  /* 调用条件处理 */
                                virtualMachineCodePointer2 = virtualMachineCodePointer;   /* 保存循环体的结束的下一个位置 */
                                gendo(jpc, 0, 0);   /* 生成条件跳转，但跳出循环的地址未知 */
                                if (sym == dosym)
                                {
                                    getsymdo;
                                }
                                else
                                {
                                    error(18);  /* 缺少do */
                                }
                                statementdo(fsys, ptx, lev);    /* 循环体 */
                                gendo(jmp, 0, virtualMachineCodePointer1); /* 回头重新判断条件 */
                                virtualCode[virtualMachineCodePointer2].a = virtualMachineCodePointer;   /* 反填跳出循环的地址，与if类似 */
                            }
                            else
                            {
                                memset(nxtlev, 0, sizeof(bool)*symnum); /* 语句结束无补救集合 */
                                testdo(fsys, nxtlev, 19);   /* 检测语句结束的正确性 */
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/*
* 表达式处理
*/
int expression(bool* fsys, int* ptx, int lev)
{
    enum symbol addop;  /* 用于保存正负号 */
    bool nxtlev[symnum];

    if(sym==plus || sym==minus) /* 开头的正负号，此时当前表达式被看作一个正的或负的项 */
    {
        addop = sym;    /* 保存开头的正负号 */
        getsymdo;
        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        termdo(nxtlev, ptx, lev);   /* 处理项 */
        if (addop == minus)
        {
            gendo(opr,0,1); /* 如果开头为负号生成取负指令 */
        }
    }
    else    /* 此时表达式被看作项的加减 */
    {
        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        termdo(nxtlev, ptx, lev);   /* 处理项 */
    }
    while (sym==plus || sym==minus)
    {
        addop = sym;
        getsymdo;
        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        termdo(nxtlev, ptx, lev);   /* 处理项 */
        if (addop == plus)
        {
            gendo(opr, 0, 2);   /* 生成加法指令 */
        }
        else
        {
            gendo(opr, 0, 3);   /* 生成减法指令 */
        }
    }
    return 0;
}

/*
* 项处理
*/
int term(bool* fsys, int* ptx, int lev)
{
    enum symbol mulop;  /* 用于保存乘除法符号 */
    bool nxtlev[symnum];

    memcpy(nxtlev, fsys, sizeof(bool)*symnum);
    nxtlev[times] = true;
    nxtlev[slash] = true;
    factordo(nxtlev, ptx, lev); /* 处理因子 */
    while(sym==times || sym==slash)
    {
        mulop = sym;
        getsymdo;
        factordo(nxtlev, ptx, lev);
        if(mulop == times)
        {
            gendo(opr, 0, 4);   /* 生成乘法指令 */
        }
        else
        {
            gendo(opr, 0, 5);   /* 生成除法指令 */
        }
    }
    return 0;
}

/*
* 因子处理
*/
int factor(bool* fsys, int* ptx, int lev)
{
    int i;
    bool nxtlev[symnum];
    testdo(facbegsys, fsys, 24);    /* 检测因子的开始符号 */
    /* while(inset(sym, facbegsys)) */  /* 循环直到不是因子开始符号 */
    if(inset(sym,facbegsys))    /* BUG: 原来的方法var1(var2+var3)会被错误识别为因子 */
    {
        if(sym == ident)    /* 因子为常量或变量 */
        {
            i = position(id, *ptx); /* 查找名字 */
            if (i == 0)
            {
                error(11);  /* 标识符未声明 */
            }
            else
            {
                switch (nameTable[i].kind)
                {
                    case constant:  /* 名字为常量 */
                        gendo(lit, 0, nameTable[i].val);    /* 直接把常量的值入栈 */
                        break;
                    case variable:  /* 名字为变量 */
                        gendo(lod, lev - nameTable[i].level, nameTable[i].adr);   // 将变量地址中的值存入栈顶
                        break;
                    case procedur:  /* 名字为过程 */
                        error(21);  /* 不能为过程 */
                        break;
                    case arrays:
                        arraycoefdo(fsys,ptx,lev);
                        gendo(lda,lev - nameTable[i].level,nameTable[i].adr);// 将数组偏移地址中的值存入栈顶
                        break;
                }
            }
            getsymdo;
        }
        else
        {
            if(sym == number)   /* 因子为数 */
            {
                if (num > amax)
                {
                    error(31);
                    num = 0;
                }
                gendo(lit, 0, num);
                getsymdo;
            }
            else
            {
                if (sym == lparen)  /* 因子为表达式 */
                {
                    getsymdo;
                    memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                    nxtlev[rparen] = true;
                    expressiondo(nxtlev, ptx, lev);
                    if (sym == rparen)
                    {
                        getsymdo;
                    }
                    else
                    {
                        error(22);  /* 缺少右括号 */
                    }
                }
                testdo(fsys, facbegsys, 23);    /* 因子后有非法符号 */
            }
        }
    }
    return 0;
}

/*
* 条件处理
*/
int condition(bool* fsys, int* ptx, int lev)
{
    enum symbol relop;
    bool nxtlev[symnum];

    if(sym == oddsym)   /* 准备按照odd运算处理 */
    {
        getsymdo;
        expressiondo(fsys, ptx, lev);
        gendo(opr, 0, 6);   /* 生成odd指令 */
    }
    else
    {
        /* 逻辑表达式处理 */
        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
        nxtlev[eql] = true;
        nxtlev[neq] = true;
        nxtlev[lss] = true;
        nxtlev[leq] = true;
        nxtlev[gtr] = true;
        nxtlev[geq] = true;
        expressiondo(nxtlev, ptx, lev);
        if (sym!=eql && sym!=neq && sym!=lss && sym!=leq && sym!=gtr && sym!=geq)
        {
            error(20);
        }
        else
        {
            relop = sym;
            getsymdo;
            expressiondo(fsys, ptx, lev);
            switch (relop)
            {
                case eql:
                    gendo(opr, 0, 8);
                    break;
                case neq:
                    gendo(opr, 0, 9);
                    break;
                case lss:
                    gendo(opr, 0, 10);
                    break;
                case geq:
                    gendo(opr, 0, 11);
                    break;
                case gtr:
                    gendo(opr, 0, 12);
                    break;
                case leq:
                    gendo(opr, 0, 13);
                    break;
            }
        }
    }
    return 0;
}

int arraydeclaration(int* ptx, int lev, int* pdx)
{
    /* 暂存数组标识名,避免被覆盖 */
    char arrayId[al];
    int constId;                  /* 常量标识符的位置 */
    int arrBase = -1, arrTop = -1;  /* 数组下界、上界的数值 */
    getsymdo;
    if(sym == lparen) {  /* 标识符之后是'(',则识别为数组 */
        strcpy(arrayId, id);
        /* 检查下界 */
        getsymdo;
        if(sym == ident) {
            constId = position(id, (*ptx));
            if(constId != 0) {
                if(nameTable[constId].kind == constant) {
                    arrBase = nameTable[constId].val;
                }
            }
        } else {
            if(sym == number) {
                arrBase = num;
            }
        }
        if(arrBase == -1) {
            error(50);
            return -1;
        }
        /* 检查冒号 */
        getsymdo;
        if(sym != colon) {
            error(50);
            return -1;
        }
        /* 检查上界 */
        getsymdo;
        if(sym == ident) {
            constId = position(id, (*ptx));
            if(constId != 0) {
                if (nameTable[constId].kind == constant) {
                    arrTop = nameTable[constId].val;
                }
            }
        } else {
            if(sym == number) {
                arrTop = num;
            }
        }
        if(arrTop == -1) { //数组上界不是数字或者常量
            error(50);
            return -1;
        }

        /* 检查')' */
        getsymdo;
        if(sym != rparen) {
            error(50);
            return -1;
        }

        /* 上下界是否符合条件检查 */

        g_arrSize = arrTop - arrBase + 1;
        g_arrBase = arrBase;

        if(g_arrSize <= 0) {
            error(50);
            return -1;
        }

        /* 恢复数组的标识符 */
        strcpy(id, arrayId);
        return 1;
    }
    return 0;
}

// 计算偏移地址
int arraycoef(bool *fsys, int *ptx, int lev)
{
    bool nxtlev[symnum];
    int i = position(id,*ptx);
    getsymdo;
    if (sym == lparen) { /* 索引是括号内的表达式 */
        getsymdo;
        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
        nxtlev[rparen] = true;
        expressiondo(nxtlev, ptx, lev);
        if (sym == rparen) { //成功处理完数组中的表达式（此时执行玩指令后值在栈顶，其即要访问的数组下标）
            gendo(lit, 0, nameTable[i].data);//数组下界放在栈顶，此时数组下标在次栈顶
            gendo(opr, 0, 3); //数组下标减去数组下界，结果放在新栈顶，得到数组地址空间的偏移地址
            return 0;
        } else {
            error(22);  /* 缺少右括号 */
        }
    } else {
        error(51);  /* 数组访问错误 */
    }
    return -1;
}

/*
* 解释程序
*/
void interpret()
{
    int p, b, t;    /* 指令指针，指令基址，栈顶指针 */
    struct instruction i;   /* 存放当前指令 */
    int s[stacksize];   /* 栈 */

    printf("start pl0\n");
    t = 0;
    b = 0;
    p = 0;
    s[0] = s[1] = s[2] = 0;
    do {
        i = virtualCode[p];    /* 读当前指令 */
        p++;
        switch (i.f)
        {
            case lit:   /* 将a的值取到栈顶 */
                s[t] = i.a;
                t++;
                break;
            case opr:   /* 数学、逻辑运算 */
                switch (i.a)
                {
                    case 0:
                        t = b;
                        p = s[t+2];
                        b = s[t+1];
                        break;
                    case 1:
                        s[t-1] = -s[t-1];
                        break;
                    case 2:
                        t--;
                        s[t-1] = s[t-1]+s[t];
                        break;
                    case 3:
                        t--;
                        s[t-1] = s[t-1]-s[t];
                        break;
                    case 4:
                        t--;
                        s[t-1] = s[t-1]*s[t];
                        break;
                    case 5:
                        t--;
                        s[t-1] = s[t-1]/s[t];
                        break;
                    case 6:
                        s[t-1] = s[t-1]%2;
                        break;
                    case 8:
                        t--;
                        s[t-1] = (s[t-1] == s[t]);
                        break;
                    case 9:
                        t--;
                        s[t-1] = (s[t-1] != s[t]);
                        break;
                    case 10:
                        t--;
                        s[t-1] = (s[t-1] < s[t]);
                        break;
                    case 11:
                        t--;
                        s[t-1] = (s[t-1] >= s[t]);
                        break;
                    case 12:
                        t--;
                        s[t-1] = (s[t-1] > s[t]);
                        break;
                    case 13:
                        t--;
                        s[t-1] = (s[t-1] <= s[t]);
                        break;
                    case 14:
                        printf("%d", s[t-1]);
                        fprintf(fa2, "%d", s[t-1]);
                        t--;
                        break;
                    case 15:
                        printf("\n");
                        fprintf(fa2,"\n");
                        break;
                    case 16:
                        printf("?");
                        fprintf(fa2, "?");
                        scanf("%d", &(s[t]));
                        fprintf(fa2, "%d\n", s[t]);
                        t++;
                        break;
                }
                break;
            case lod:   /* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
                s[t] = s[base(i.l,s,b)+i.a];
                t++;
                break;
            case sto:   /* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
                t--;
                s[base(i.l, s, b) + i.a] = s[t];
                break;

            case lda: /* 数组元素访问,当前栈顶为元素索引,执行后,栈顶变成元素的值*/
                s[t-1] = s[base(i.l, s, b) + i.a + s[t-1]];
                break;
            case sta: /* 栈顶的值存到数组中,索引为次栈顶*/
                t -= 2;
                s[base(i.l, s, b) + i.a + s[t]] = s[t+1];
                break;

            case cal:   /* 调用子过程 */
                s[t] = base(i.l, s, b); /* 将父过程基地址入栈 */
                s[t+1] = b; /* 将本过程基地址入栈，此两项用于base函数 */
                s[t+2] = p; /* 将当前指令指针入栈 */
                b = t;  /* 改变基地址指针值为新过程的基地址 */
                p = i.a;    /* 跳转 */
                break;
            case inte:  /* 分配内存 */
                t += i.a;
                break;
            case jmp:   /* 直接跳转 */
                p = i.a;
                break;
            case jpc:   /* 条件跳转 */
                t--;
                if (s[t] == 0)
                {
                    p = i.a;
                }
                break;
        }
    } while (p != 0);
}

/* 通过过程基址求上l层过程的基址 */
int base(int l, int* s, int b)
{
    int b1;
    b1 = b;
    while (l > 0)
    {
        b1 = s[b1];
        l--;
    }
    return b1;
}

