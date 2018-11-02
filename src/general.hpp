#pragma once
#include<string>
#include<vector>

using std::string;
using std::vector;

enum{
	ISS = 1,
	EXEC = 2,
	WRITE = 3
};

enum{
	BUSY = 1,
	OP = 2,
	VJ = 3,
	VK = 4,
	QJ = 5,
	QK = 6,
	A = 7
};


vector<string> instruction_split(string p);
