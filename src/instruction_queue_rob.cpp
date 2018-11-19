#include "instruction_queue_rob.hpp"
#include "general.hpp"

instruction_queue_rob::instruction_queue_rob(sc_module_name name, vector<string> inst_q, nana::listbox &instr):
sc_module(name),
instruct_queue(inst_q),
instructions(instr)
{
	SC_THREAD(main);
	sensitive << in;
	dont_initialize();
	SC_METHOD(leitura_rob);
	sensitive << in_rob;
	dont_initialize();
}

void instruction_queue_rob::main()
{
	auto cat = instructions.at(0);
	for(pc = 0; pc < instruct_queue.size() ; pc++)
	{
		cat.at(pc).text(ISS,"X");
		cat.at(pc).text(EXEC,"");
		cat.at(pc).text(WRITE,"");
		out->write(instruct_queue[pc] + " " + std::to_string(pc));
		wait();
	}
}

void instruction_queue_rob::leitura_rob()
{
	string p;
	vector<string> ord;
	in_rob->read(p);
	ord = instruction_split(p);
	if(ord[0] == "R")
		pc = last_pc;
	else if(ord[0] == "S" && ord.size() == 1)
		last_pc = pc;
	else if(ord[0] == "S")
	{
		last_pc = pc;
		pc += std::stoi(ord[1]);
	}
	else
		pc = last_pc + std::stoi(p);
}