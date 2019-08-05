#include "instruction_queue.hpp"
#include "general.hpp"

instruction_queue::instruction_queue(sc_module_name name, vector<string> inst_q, nana::listbox &instr):
sc_module(name),
instruct_queue(inst_q),
instructions(instr)
{
	SC_THREAD(main);
	sensitive << in;
	dont_initialize();
}

void instruction_queue::main()
{
	auto cat = instructions.at(0);
	for(pc = 0; pc < instruct_queue.size() ; pc++)
	{
		cat.at(pc).text(ISS,"X");
		out->write(instruct_queue[pc] + " " + std::to_string(pc));
		wait();
	}
}
