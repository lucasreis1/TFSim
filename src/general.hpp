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
	BUSY = 2,
	OP = 3,
	VJ = 4,
	VK = 5,
	QJ = 6,
	QK = 7,
	A = 8
};


vector<string> instruction_split(string p);
