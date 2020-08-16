#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <iostream>
using namespace std;
#include "TimerHandler.h"
#include "TerminalHandler.h"
#define R7 65534
#define SP 65532
#define R0 65520
#define PSW 65518
#define STACK_SPACE 65279
#define STACK_BOUND 60279

class Processor
{
public:
    Processor(unsigned short main);
    void executeProgram();

private:
    void fetch();
    int decode(int num, int index);
    void execute();
    void handleInterrupts();
    int getImmOperand(int num, int index);
    int getRegDirOperand(int num, int index);
    int getRegIndOperand(int num, int index);
    int getRegIndPomOperand(int num, int index);
    int getApsoluteOperand(int num, int index);
    short readOperandFromMemory(unsigned short address, int amount, int high);
    void iret();
    void ret();
    void intInstr();
    void call();
    void jmp();
    void jeq();
    void jne();
    void jgt();
    void push();
    void pop();
    void xchg();
    void mov();
    void add();
    void sub();
    void mul();
    void div();
    void cmp();
    void notInstr();
    void andInstr();
    void orInstr();
    void xorInstr();
    void test();
    void shl();
    void shr();
    void setOMinus(short result);
    void setZAndN(short result);
    void setO(short result);
    char numOfOperands;
    char operandSize;
    short firstOperand;
    short secondOperand;
    unsigned short firstOperandAddress;
    unsigned short secondOperandAddress;
    unsigned char instructionCode;
    unsigned short PSWRegister = 0;
    bool finished = false;
    bool wrongInstructionCode = false;
    Memory memory;
    unsigned short currentPC;
    TimerHandler myTimerHandler;
    TerminalHandler myTerminalHandler;
    unsigned char instructionBytes[7];
};
#endif