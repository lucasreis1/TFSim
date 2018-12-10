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
		if(pc)
			cat.at(pc-1).select(false);
		cat.at(pc).select(true,true);
		cat.at(pc).text(ISS,"X");
		cat.at(pc).text(EXEC,"");
		cat.at(pc).text(WRITE,"");
		out->write(instruct_queue[pc] + " " + std::to_string(pc));
		wait();
	}
}

void instruction_queue_rob::leitura_rob()
{
	bool pc_moved = true;
	string p;
	vector<string> ord;
	unsigned int old_pc;
	in_rob->read(p);
	ord = instruction_split(p);
	auto cat = instructions.at(0);
	if(ord[0] == "R")
	{
		cat.at(pc).select(false);
		old_pc = pc;
		pc = last_pc;
	}
	else if(ord[0] == "S" && ord.size() == 1)
	{
		last_pc = pc;
		pc_moved = false;
	}
	else if(ord[0] == "S")
	{
		last_pc = pc;
		cat.at(pc).select(false);
		old_pc = pc;
		pc += std::stoi(ord[1]);
	}
	else
	{
		cat.at(pc).select(false);
		old_pc = pc;
		pc = last_pc + std::stoi(p);
	}
	if(pc_moved && pc < old_pc)
		clear_gui(pc+1);
}

void instruction_queue_rob::clear_gui(unsigned int pos)
{
	auto cat = instructions.at(0);
	for(unsigned int i = pos ; i < instruct_queue.size() ; i++)
	{
		cat.at(i).text(ISS,"");
		cat.at(i).text(EXEC,"");
		cat.at(i).text(WRITE,"");
	}
}