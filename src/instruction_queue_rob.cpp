#include "instruction_queue_rob.hpp"
#include "general.hpp"

instruction_queue_rob::instruction_queue_rob(sc_module_name name, vector<string> inst_q,int rb_sz, nana::listbox &instr):
sc_module(name),
instruct_queue(inst_q),
original_instruct(inst_q),
last_instr(rb_sz),
last_pc(rb_sz),
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
	pc = 0;
	while(1)
	{
		if(pc < instruct_queue.size())
		{
			if(pc)
				cat.at(pc-1).select(false);
			cat.at(pc).select(true,true);
			cat.at(pc).text(ISS,"X");
			cat.at(pc).text(EXEC,"");
			cat.at(pc).text(WRITE,"");
			out->write(instruct_queue[pc] + " " + std::to_string(pc));
			pc++;
		}

		wait();
	}
}

void instruction_queue_rob::leitura_rob()
{
//	bool pc_moved = true;
//	string p;
//	vector<string> ord;
//	vector<unsigned int> old_pc(last_pc.size());
//	unsigned int index;
//	in_rob->read(p);
//	ord = instruction_split(p);
//	auto cat = instructions.at(0);
//	index = std::stoi(ord[1]) - 1;
//	if(ord[0] == "R")
//	{
//		cat.at(pc).select(false);
//		old_pc[index] = pc;
//		pc = last_pc[index];
//	}
//	else if(ord[0] == "S" && ord.size() == 2)
//	{
//		last_pc[index] = pc - 1;
//		pc_moved = false;
//	}
//	else if(ord[0] == "S")
//	{
//		last_pc[index] = pc - 1;
//		cat.at(pc-1).select(false);
//		old_pc[index] = pc;
//		pc += std::stoi(ord[2]) - 1;
//	}
//	else
//	{
//		cat.at(pc-1).select(false);
//		old_pc[index] = pc;
//		pc = last_pc[index] + std::stoi(p);
//	}
//	if(pc_moved && pc < old_pc[index])
//		clear_gui(pc+1);
	string p;
	vector<string> ord;
	unsigned int index;
	int offset;
	in_rob->read(p);
	ord = instruction_split(p);
	index = std::stoi(ord[1])-1;
	cout << "######" << p << endl;
	if(ord[0] == "R") //reverter salto incorreto
	{
		replace_instructions(last_pc[index]+1,index);		
		pc = last_pc[index];
		instruct_queue = last_instr[index]; 
	}	
	else if(ord[0] == "S" && ord.size() == 3) //realiza salto (especulado) e armazena informacoes pre-salto
	{
		last_instr[index] = instruct_queue;
		last_pc[index] = pc;
		vector<string> instructions;
		offset = std::stoi(ord[2]);
		for(unsigned int i = pc+offset ; i < original_instruct.size(); i++)
			instructions.push_back(original_instruct[i]);
		add_instructions(pc,instructions);
	}
	else if(ord[0] == "S") //armazena informacoes pre-salto mas nao salta
	{
		last_instr[index] = instruct_queue;	
		last_pc[index] = pc;
	}
	else //salta atrasado (quando foi predito que nao saltaria)
	{
		vector<string> instructions;
		offset = std::stoi(p);
		for(unsigned int i = pc+offset ; i < original_instruct.size() ; i++)
			instructions.push_back(original_instruct[i]);
		add_instructions(pc,instructions);
	}
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

void instruction_queue_rob::replace_instructions(unsigned int pos,unsigned int index)
{
	auto cat = instructions.at(0);
	unsigned int sz = instruct_queue.size();
	unsigned int i;
	for(i = pos ; i < last_instr.size() ; i++)
	{
		if(i < sz)
		{
			instruct_queue[i] = last_instr[index][i];
			cat.at(i).text(ISS,"");
			cat.at(i).text(EXEC,"");
			cat.at(i).text(WRITE,"");
			cat.at(i).text(INSTR,last_instr[index][i]);
		}
		else
		{
			instruct_queue.push_back(last_instr[index][i]);
			instructions.at(0).append(last_instr[index][i]);
		}

	}
	auto item_proxy = instructions.at(0).at(i);
	for(; i < instruct_queue.size() ; i++)
	{
		instruct_queue.pop_back();	
		item_proxy = instructions.erase(item_proxy);
	}

}

void instruction_queue_rob::add_instructions(unsigned int pos, vector<string> instr_vec)
{
	unsigned int sz = instruct_queue.size();
	auto cat = instructions.at(0);
	for(unsigned int i = pos ; i < pos + instr_vec.size() ; i++)
	{
		if(i < sz)
		{
			instruct_queue[i] = instr_vec[i];	
			cat.at(i).text(ISS,"");
			cat.at(i).text(EXEC,"");
			cat.at(i).text(WRITE,"");
			cat.at(i).text(INSTR,instr_vec[i]);
		}
		else
		{
			instruct_queue.push_back(instr_vec[i]);
			instructions.at(0).append(instr_vec[i]);
		}
	}
}
