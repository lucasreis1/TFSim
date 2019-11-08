#pragma once
#include<string>
#include<vector>

using std::string;
using std::vector;

enum{
    INSTR = 0,
    ISS = 1,
    EXEC = 2,
    WRITE = 3
};

enum{
    BUSY = 2,
    OP = 3,
    VJ = 4,
    VK = 5,
    QJ = 6,
    QK = 7,
    A = 8
};

enum{
    R_BUSY = 1,
    INSTRUCTION = 2,
    STATE = 3,
    DESTINATION = 4,
    VALUE = 5
};

vector<string> instruction_split(string p);
