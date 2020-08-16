#include "Processor.h"
#include "Memory.h"
#include "AccessViolation.h"

#define Z 1
#define N 8
#define C 4
#define O 2

Processor::Processor(unsigned short main)
{
    Memory::kernelTime = true;
    currentPC = readOperandFromMemory(0, 2, 0);
    memory.write((main >> 8) & 255, STACK_SPACE);
    memory.write(main & 255, STACK_SPACE - 1);
    PSWRegister |= (1 << 15);
    memory.write((PSWRegister >> 8) & 255, STACK_SPACE - 2);
    memory.write(PSWRegister & 255, STACK_SPACE - 3);
    memory.write((STACK_SPACE - 3) & 255, SP);
    memory.write(((STACK_SPACE - 3) >> 8) & 255, SP + 1);
}

void Processor::executeProgram()
{
    while (!finished)
    {
        fetch();
        if (!wrongInstructionCode)
        {
            int nextIndex;
            if (numOfOperands != 0)
                nextIndex = decode(1, 1);
            if (numOfOperands == 2)
                decode(2, nextIndex);
            execute();
        }
        handleInterrupts();
    }
}

void Processor::fetch()
{

    unsigned short data = memory[currentPC];
    currentPC = currentPC + 1;
    operandSize = data & 4 ? 2 : 1;
    instructionCode = ((data >> 3) & 31);
    if (instructionCode >= 0 && instructionCode <= 2)
    {
        numOfOperands = 0;
    }
    else
    {
        if (instructionCode >= 3 && instructionCode <= 10)
        {
            numOfOperands = 1;
        }
        else
        {
            numOfOperands = 2;
        }
    }

    if (instructionCode > 24)
    {
        wrongInstructionCode = true;
        return;
    }
    int number = 0;
    if (numOfOperands != 0)
    {
        instructionBytes[number++] = instructionCode;
        instructionBytes[number++] = memory[currentPC];
        currentPC = currentPC + 1;
        if ((((instructionBytes[1] >> 5) & 7) == 0 && operandSize == 2) ||
            (((instructionBytes[1] >> 5) & 7) == 3) || ((instructionBytes[1] >> 5) & 7) == 4)
        {
            instructionBytes[number++] = memory[currentPC++];
            instructionBytes[number++] = memory[currentPC++];
        }
        else
        {
            if (((instructionBytes[1] >> 5) & 7) == 0 && operandSize == 1)
            {

                instructionBytes[number++] = memory[currentPC++];
            }
        }
    }
    if (numOfOperands == 2)
    {
        instructionBytes[number] = memory[currentPC++];

        if ((((instructionBytes[number] >> 5) & 7) == 0 && operandSize == 2) ||
            (((instructionBytes[number] >> 5) & 7) == 3) || ((instructionBytes[number] >> 5) & 7) == 4)
        {
            number++;
            instructionBytes[number++] = memory[currentPC++];
            instructionBytes[number++] = memory[currentPC++];
        }
        else
        {
            if (((instructionBytes[number] >> 5) & 7) == 0 && operandSize == 1)
            {
                number++;
                instructionBytes[number++] = memory[currentPC++];
            }
        }
    }
    memory.write(currentPC & 255, R7);
    memory.write((currentPC >> 8) & 255, R7 + 1);
}

int Processor::decode(int num, int index)
{
    unsigned char firstOpInfo = instructionBytes[index];
    unsigned char addressing;
    switch ((firstOpInfo >> 5) & 7)
    {
    case 0:
        getImmOperand(num, index);
        break;
    case 1:
        getRegDirOperand(num, index);
        break;
    case 2:
        getRegIndOperand(num, index);
        break;
    case 3:
        getRegIndPomOperand(num, index);
        break;
    case 4:
        getApsoluteOperand(num, index);
        break;
    default:
        throw "Invalid addressing error";
        break;
    }
}

int Processor::getImmOperand(int num, int index)
{
    short operand = 0;
    unsigned char lowByte = instructionBytes[++index];
    operand |= lowByte;
    if (operandSize == 2)
    {
        unsigned short highByte = instructionBytes[++index];
        operand |= (highByte << 8);
    }
    if (num == 1)
        firstOperand = operand;

    else
    {
        secondOperand = operand;
    }
    return ++index;
}

int Processor::getRegDirOperand(int num, int index)
{

    unsigned short operandAddress = R0 + ((instructionBytes[index] & (15 << 1)) >> 1) * 2;
    int amount = operandSize;
    int high = instructionBytes[index] & 1;

    short operand = readOperandFromMemory(operandAddress, amount, high);
    if (high)
    {
        operandAddress += 1;
    }
    if (num == 1)
    {
        firstOperand = operand;
        firstOperandAddress = operandAddress;
    }
    else
    {
        secondOperand = operand;
        secondOperandAddress = operandAddress;
    }
    return ++index;
}

int Processor::getRegIndOperand(int num, int index)
{
    unsigned short adress = R0 + ((instructionBytes[index] & (15 << 1)) >> 1) * 2;
    unsigned short operandAddress = readOperandFromMemory(adress, 2, 0);
    short operand = readOperandFromMemory(operandAddress, operandSize, 0);
    if (num == 1)
    {
        firstOperandAddress = operandAddress;
        firstOperand = operand;
    }
    else
    {
        secondOperandAddress = operandAddress;
        secondOperand = operand;
    }
    return ++index;
}
int Processor::getRegIndPomOperand(int num, int index)
{
    unsigned short adress = R0 + ((instructionBytes[index] & (15 << 1)) >> 1) * 2;
    short operandAddress = readOperandFromMemory(adress, 2, 0);
    unsigned char immLow = instructionBytes[++index];
    unsigned short immHigh = instructionBytes[++index];
    unsigned short immValue = immLow;
    immValue |= (immHigh << 8);
    unsigned short myOpAddress = operandAddress + immValue;
    short operand = readOperandFromMemory(myOpAddress, operandSize, 0);
    if (num == 1)
    {
        firstOperand = operand;
        firstOperandAddress = myOpAddress;
    }
    else
    {
        secondOperand = operand;
        secondOperandAddress = myOpAddress;
    }
    return ++index;
}

int Processor::getApsoluteOperand(int num, int index)
{
    unsigned short low = instructionBytes[++index];
    unsigned short operandAddress = low;
    unsigned short high = instructionBytes[++index];
    operandAddress |= (high << 8);
    short operand = readOperandFromMemory(operandAddress, operandSize, 0);
    if (num == 1)
    {
        firstOperandAddress = operandAddress;
        firstOperand = operand;
    }
    else
    {
        secondOperandAddress = operandAddress;
        secondOperand = operand;
    }
    return ++index;
}

void Processor::execute()
{
    switch (instructionCode)
    {
    case 0:
        finished = true;
        break;
    case 1:
        iret();
        break;
    case 2:
        ret();
        break;
    case 3:
        intInstr();
        break;
    case 4:
        call();
        break;
    case 5:
        jmp();
        break;
    case 6:
        jeq();
        break;
    case 7:
        jne();
        break;
    case 8:
        jgt();
        break;
    case 9:
        push();
        break;
    case 10:
        pop();
        break;
    case 11:
        xchg();
        break;
    case 12:
        mov();
        break;
    case 13:
        add();
        break;
    case 14:
        sub();
        break;
    case 15:
        mul();
        break;
    case 16:
        div();
        break;
    case 17:
        cmp();
        break;
    case 18:
        notInstr();
        break;
    case 19:
        andInstr();
        break;
    case 20:
        orInstr();
        break;
    case 21:
        xorInstr();
        break;
    case 22:
        test();
        break;
    case 23:
        shl();
        break;
    case 24:
        shr();
        break;
    default:
        throw "Wrong operation code";
        break;
    }
}
short Processor::readOperandFromMemory(unsigned short address, int amount, int high)
{

    short operand;
    if (high && amount == 1)
    {
        operand = memory[address + 1];
        return operand;
    }
    operand = memory[address];
    if (amount == 2)
    {
        unsigned short temp = memory[address + 1];
        operand |= (temp << 8);
    }
    return operand;
}

void Processor::iret()
{
    unsigned short stackAddress = readOperandFromMemory(SP, 2, 0);
    PSWRegister = readOperandFromMemory(stackAddress, 2, 0);
    stackAddress += 2;
    currentPC = readOperandFromMemory(stackAddress, 2, 0);
    stackAddress += 2;
    memory.write(stackAddress & 255, SP);
    memory.write((stackAddress >> 8) & 255, SP + 1);
    memory.write(PSWRegister & 255, PSW);
    memory.write((PSWRegister >> 8) & 255, PSW + 1);
    memory.write(currentPC & 255, R7);
    memory.write((currentPC >> 8) & 255, R7 + 1);
    Memory::kernelTime = false;
}

void Processor::ret()
{
    unsigned short stackAddress = readOperandFromMemory(SP, 2, 0);
    currentPC = readOperandFromMemory(stackAddress, 2, 0);
    stackAddress += 2;
    memory.write(stackAddress & 255, SP);
    memory.write((stackAddress >> 8) & 255, SP + 1);
    memory.write(currentPC & 255, R7);
    memory.write((currentPC >> 8) & 255, R7 + 1);
}

void Processor::intInstr()
{
    Memory::kernelTime = true;
    unsigned short stackAddress = readOperandFromMemory(SP, 2, 0);
    if ((stackAddress - 4) < STACK_BOUND)
    {
        throw new AccessViolationException();
    }
    memory.write((currentPC >> 8) & 255, stackAddress - 1);
    memory.write(currentPC & 255, stackAddress - 2);
    stackAddress -= 2;
    memory.write((PSWRegister >> 8) & 255, stackAddress - 1);
    memory.write(PSWRegister & 255, stackAddress - 2);
    stackAddress -= 2;
    memory.write(stackAddress & 255, SP);
    memory.write((stackAddress >> 8) & 255, SP + 1);
    currentPC = readOperandFromMemory((firstOperand % 8) * 2, 2, 0);
}
void Processor::call()
{
    unsigned short stackAddress = readOperandFromMemory(SP, 2, 0);
    memory.write((currentPC >> 8) & 255, stackAddress - 1);
    memory.write(currentPC & 255, stackAddress - 2);
    stackAddress -= 2;
    memory.write(stackAddress & 255, SP);
    memory.write((stackAddress >> 8) & 255, SP + 1);
    currentPC = firstOperand;
}

void Processor::jmp()
{
    currentPC = firstOperand;
}

void Processor::jeq()
{
    if (PSWRegister & 1)
    {
        currentPC = firstOperand;
    }
}
void Processor::jne()
{
    if (!(PSWRegister & 1))
    {
        currentPC = firstOperand;
    }
}
void Processor::jgt()
{
    if (!(PSWRegister & (1 << 3) || PSWRegister & (2)) && !(PSWRegister & 1))
    {
        currentPC = firstOperand;
    }
}

void Processor::push()
{
    unsigned short stackAddress = readOperandFromMemory(SP, 2, 0);
    if ((stackAddress - 2) < STACK_BOUND)
    {
        throw new AccessViolationException();
    }
    memory.write((firstOperand >> 8) & 255, stackAddress - 1);
    memory.write(firstOperand & 255, stackAddress - 2);
    stackAddress -= 2;
    memory.write(stackAddress & 255, SP);
    memory.write((stackAddress >> 8) & 255, SP + 1);
}

void Processor::pop()
{
    unsigned short stackAddress = readOperandFromMemory(SP, 2, 0);
    short value = readOperandFromMemory(stackAddress, 2, 0);
    memory.write(value & 255, firstOperandAddress);
    memory.write((value >> 8) & 255, firstOperandAddress + 1);
}
void Processor::xchg()
{
    memory.write(firstOperand & 255, secondOperandAddress);
    memory.write(secondOperand & 255, firstOperandAddress);
    if (operandSize == 2)
    {
        memory.write((firstOperand >> 8) & 255, secondOperandAddress + 1);
        memory.write((secondOperand >> 8) & 255, firstOperandAddress + 1);
    }
}

void Processor::mov()
{
    memory.write(firstOperand & 255, secondOperandAddress);
    if (operandSize == 2)
    {
        memory.write((firstOperand >> 8) & 255, secondOperandAddress + 1);
    }
    setZAndN(firstOperand);
    memory.write(PSWRegister & 255, PSW);
    memory.write((PSWRegister >> 8) & 255, PSW + 1);
}
void Processor::add()
{
    short result = firstOperand + secondOperand;
    memory.write(result & 255, secondOperandAddress);
    if (operandSize == 2)
    {
        memory.write((result >> 8) & 255, secondOperandAddress + 1);
    }
    setZAndN(result);
    setO(result);
    PSWRegister &= ~C;
    memory.write(PSWRegister & 255, PSW);
    memory.write((PSWRegister >> 8) & 255, PSW + 1);
}
void Processor::sub()
{
    short result = secondOperand - firstOperand;
    memory.write(result & 255, secondOperandAddress);
    if (operandSize == 2)
    {
        memory.write((result >> 8) & 255, secondOperandAddress + 1);
    }
    setZAndN(result);
    setOMinus(result);
    PSWRegister &= ~C;
    memory.write(PSWRegister & 255, PSW);
    memory.write((PSWRegister >> 8) & 255, PSW + 1);
}
void Processor::mul()
{
    short result = secondOperand * firstOperand;
    memory.write(result & 255, secondOperandAddress);
    if (operandSize == 2)
    {
        memory.write((result >> 8) & 255, secondOperandAddress + 1);
    }
    setZAndN(result);
    memory.write(PSWRegister & 255, PSW);
    memory.write((PSWRegister >> 8) & 255, PSW + 1);
}
void Processor::div()
{
    short result = secondOperand / firstOperand;
    memory.write(result & 255, secondOperandAddress);
    if (operandSize == 2)
    {
        memory.write((result >> 8) & 255, secondOperandAddress + 1);
    }
    setZAndN(result);
    memory.write(PSWRegister & 255, PSW);
    memory.write((PSWRegister >> 8) & 255, PSW + 1);
}
void Processor::cmp()
{
    short temp = secondOperand - firstOperand;
    setZAndN(temp);
    setOMinus(temp);
    PSWRegister &= ~C;
    memory.write(PSWRegister & 255, PSW);
    memory.write((PSWRegister >> 8) & 255, PSW + 1);
}
void Processor::notInstr()
{
    short result = ~firstOperand;
    memory.write(result & 255, secondOperandAddress);
    if (operandSize == 2)
    {
        memory.write((result >> 8) & 255, secondOperandAddress + 1);
    }
    setZAndN(result);
    memory.write(PSWRegister & 255, PSW);
    memory.write((PSWRegister >> 8) & 255, PSW + 1);
}
void Processor::andInstr()
{
    short result = secondOperand & firstOperand;
    memory.write(result & 255, secondOperandAddress);
    if (operandSize == 2)
    {
        memory.write((result >> 8) & 255, secondOperandAddress + 1);
    }
    setZAndN(result);
    memory.write(PSWRegister & 255, PSW);
    memory.write((PSWRegister >> 8) & 255, PSW + 1);
}
void Processor::orInstr()
{
    short result = secondOperand | firstOperand;
    memory.write(result & 255, secondOperandAddress);
    if (operandSize == 2)
    {
        memory.write((result >> 8) & 255, secondOperandAddress + 1);
    }
    setZAndN(result);
    memory.write(PSWRegister & 255, PSW);
    memory.write((PSWRegister >> 8) & 255, PSW + 1);
}
void Processor::xorInstr()
{
    short result = secondOperand ^ firstOperand;
    memory.write(result & 255, secondOperandAddress);
    if (operandSize == 2)
    {
        memory.write((result >> 8) & 255, secondOperandAddress + 1);
    }
    setZAndN(result);
    memory.write(PSWRegister & 255, PSW);
    memory.write((PSWRegister >> 8) & 255, PSW + 1);
}
void Processor::test()
{
    short result = firstOperand & secondOperand;
    setZAndN(result);
    memory.write(PSWRegister & 255, PSW);
    memory.write((PSWRegister >> 8) & 255, PSW + 1);
}

void Processor::shl()
{
    short result = secondOperand << firstOperand;
    memory.write(result & 255, secondOperandAddress);
    if (operandSize == 2)
    {
        memory.write((result >> 8) & 255, secondOperandAddress + 1);
    }
    setZAndN(result);
    int offset = operandSize * 8 - 1;
    memory.write(PSWRegister & 255, PSW);
    memory.write((PSWRegister >> 8) & 255, PSW + 1);
    int mask = 1 << offset;
    bool setC = false;
    for (int i = 0; i < firstOperand; i++)
    {
        if (secondOperand & (mask >> i))
        {
            setC = true;
            break;
        }
    }
    if (setC)
    {
        PSWRegister |= C;
    }
    else
    {
        PSWRegister &= ~C;
    }
}

void Processor::shr()
{
    short result = secondOperand >> firstOperand;
    memory.write(result & 255, secondOperandAddress);
    if (operandSize == 2)
    {
        memory.write((result >> 8) & 255, secondOperandAddress + 1);
    }
    setZAndN(result);
    bool setC = false;
    for (int i = 0; i < firstOperand; i++)
    {
        if (secondOperand & (1 << i))
        {
            setC = true;
            break;
        }
    }
    if (setC)
    {
        PSWRegister |= C;
    }
    else
    {
        PSWRegister &= ~C;
    }
    memory.write(PSWRegister & 255, PSW);
    memory.write((PSWRegister >> 8) & 255, PSW + 1);
}
void Processor::setZAndN(short result)
{
    if (result == 0)
    {
        PSWRegister |= Z;
    }
    else
    {
        PSWRegister &= ~Z;
    }
    if (result < 0)
    {
        PSWRegister |= N;
    }
    else
    {
        PSWRegister &= ~N;
    }
}
void Processor::setO(short result)
{
    int offset = operandSize * 8 - 1;
    if (((firstOperand & (1 << offset)) && (secondOperand & (1 << offset)) && !(result & (1 << offset))) || (!(firstOperand & (1 << offset)) && !(secondOperand & (1 << offset)) && (result & (1 << offset))))
    {
        PSWRegister |= O;
    }
    else
    {
        PSWRegister &= ~O;
    }
}

void Processor::setOMinus(short result)
{
    int offset = operandSize * 8 - 1;
    if (((firstOperand & (1 << offset)) && !(secondOperand & (1 << offset)) && !(result & (1 << offset))) || (!(firstOperand & (1 << offset)) && (secondOperand & (1 >> offset)) && (result & (1 << offset))))
    {
        PSWRegister |= O;
    }
    else
    {
        PSWRegister &= ~O;
    }
}

void ::Processor::handleInterrupts()
{
    bool interruptCondition = false;
    int entryNumber = 0;
    if (!finished)
    {
        if (PSWRegister & (1 << 15))
        {
            if (wrongInstructionCode)
            {
                interruptCondition = true;
                entryNumber = 1;
            }
            else
            {
                if (!(PSWRegister & (1 << 14)) && myTerminalHandler.checkIf())
                {
                    entryNumber = 3;
                    interruptCondition = true;
                }
                else
                {
                    if (!(PSWRegister & (1 << 13)) && myTimerHandler.checkIfTime())
                    {
                        entryNumber = 2;
                        interruptCondition = true;
                    }
                }
            }
        }
        if (interruptCondition)
        {
            Memory::kernelTime = true;
            unsigned short stackAddress = readOperandFromMemory(SP, 2, 0);
            if ((stackAddress - 4) < STACK_BOUND)
            {
                throw new AccessViolationException();
            }
            memory.write((currentPC >> 8) & 255, stackAddress - 1);
            memory.write(currentPC & 255, stackAddress - 2);
            memory.write((PSWRegister >> 8) & 255, stackAddress - 3);
            memory.write(PSWRegister & 255, stackAddress - 4);
            stackAddress -= 4;
            memory.write(stackAddress & 255, SP);
            memory.write((stackAddress >> 8) & 255, SP + 1);
            currentPC = readOperandFromMemory(entryNumber * 2, 2, 0);
            Memory::kernelTime = true;
        }
    }
    for (int i = 0; i < 7; i++)
        instructionBytes[i] = 0;
}