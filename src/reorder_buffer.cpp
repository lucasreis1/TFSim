#include "reorder_buffer.hpp"

reorder_buffer::reorder_buffer(sc_module_name name,unsigned int t,unsigned int t2): sc_module(name),tam(t), preditor(t2)
{
	branch_instr = {{"BEQ",0},{"BNE",1},{"BGTZ",2},{"BLTZ",3},{"BGEZ",4},{"BLEZ",5}};
	ptrs = new rob_slot*[tam];
	for(unsigned int i = 0 ; i < tam ; i++)
		ptrs[i] = new rob_slot(i+1);
	SC_THREAD(leitura_issue);
	sensitive << in_issue;
	dont_initialize();
	SC_THREAD(new_rob_head);
	sensitive << new_rob_head_event;
	dont_initialize();
	SC_METHOD(leitura_cdb);
	sensitive << in_cdb;
	dont_initialize();
	SC_METHOD(leitura_adu);
	sensitive << in_adu;
	dont_initialize();
	SC_THREAD(check_conflict);
	sensitive << in_slb;
	dont_initialize();
}

~reorder_buffer::reorder_buffer()
{
	for(unsigned int i = 0 ; i < tam ; i++)
		delete ptrs[i];
	delete ptrs;
}

void reorder_buffer::leitura_issue()
{
	string p;
	vector<string> ord;
	int pos,regst;
	while(true)
	{
		pos = busy_check();
		if(pos == -1)
		{
			cout << "ROB esta totalmente ocupado" << endl << flush;
			wait(free_rob_event);
		}
		in_issue->read(p);
		out_issue->write(std::to_string(pos+1));
		ord = instruction_split(p);
		cout << "Inserindo instrucao " << p << " no ROB " << pos+1 <<"|" << sc_time_stamp() << endl << flush;
		ptrs[pos]->busy = true;
		ptrs[pos]->ready = false;
		ptrs[pos]->instruction = ord[0];
		ptrs[pos]->state = ISSUE;
		if(ord[0].at(0) == 'S')
		{
			regst = ask_status(true,ord[1]);
			if(regst == 0)
				ptrs[pos]->value = ask_value(true,ord[1]);
			else
				ptrs[pos]->qj = regst;
		}
		else if(ord[0].at(0) == 'B')
		{
			ptrs[pos]->destination = ord[1];
			regst = ask_status(true,ord[1]);
			if(!regst)
				ptrs[pos]->vj = ask_value(true,ord[1]);
			else
				ptrs[pos]->qj = regst;
			if(branch_instr[ord[0]] < 2) //instrucao com 2 operandos (BEQ,BNE)
			{
				regst = ask_status(true,ord[2]);
				if(!regst)
					ptrs[pos]->vk = ask_value(true,ord[2]);
				else
					ptrs[pos]->qk = regst;
				ptrs[pos]->destination = ord[3];
			}
			else
				ptrs[pos]->destination = ord[2];
			if(preditor.predict())
				out_iq->write("S " + ptrs[pos]->destination);
			else
				out_iq->write("S");
			if(ptrs[pos]->qj == 0 && ptrs[pos]->qk == 0)
				ptrs[pos]->ready = true;
		}
		else
		{
			ptrs[pos]->destination = ord[1];
			ask_status(false,ord[1],pos+1);
		}
		if(rob_buff.empty())
			new_rob_head_event.notify(1,SC_NS);
		rob_buff.push_back(ptrs[pos]);
		wait();
	}
}

void reorder_buffer::new_rob_head()
{
	unsigned int instr_type;
	bool pred;
	while(true)
	{
		if(rob_buff.empty())
			wait(new_rob_head_event);
		if(!rob_buff[0]->ready)
			wait(rob_head_value_event);
		rob_buff[0]->state = COMMIT;
		if(rob_buff[0]->instruction.at(0) == 'S')
			mem_write(std::stoi(rob_buff[0]->destination),rob_buff[0]->value,rob_buff[0]->entry);
		else if(rob_buff[0]->instruction.at(0) == 'B')
		{
			instr_type = branch_instr[rob_buff[0]->instruction];
			if(instr_type < 2)
				pred = branch(instr_type,rob_buff[0]->vj,rob_buff[0]->vk);
			else
				pred = branch(instr_type,rob_buff[0]->vj);
			if(pred != preditor.predict())
			{
				if(pred)
					out_iq->write(rob_buff[0]->destination);
				else
					out_iq->write("R");
				cout << "-----------------LIMPANDO ROB-----------------" << endl << flush;
				_flush();
				out_resv->write("F");
				out_slb->write("F");
				out_rb->write("F");
			}

		}
		else
			ask_value(false,rob_buff[0]->destination,rob_buff[0]->value);
		rob_buff[0]->busy = false;
		rob_buff[0]->ready = false;
		rob_buff[0]->destination = "";
		rob_buff[0]->qj = rob_buff[0]->qk = 0;
		cout << "Commit da instrucao " << rob_buff[0]->instruction << " com valor " << rob_buff[0]->value << " no ciclo " << sc_time_stamp() << endl << flush;
		free_rob_event.notify(1,SC_NS);
		rob_buff.pop_front();
		wait(1,SC_NS);
	}
}
void reorder_buffer::leitura_cdb()
{
	unsigned int index;
	float value;
	string p;
	vector<string> ord;
	in_cdb->read(p);
	ord = instruction_split(p);
	index = std::stoi(ord[0]);
	value = std::stof(ord[1]);
	check_dependencies(index,value);
	ptrs[index-1]->ready = true;
	ptrs[index-1]->value = value;
	ptrs[index-1]->state = WRITE;
	if(rob_buff[0]->entry == index)
		rob_head_value_event.notify(1,SC_NS);
}
void reorder_buffer::leitura_adu()
{
	string p;
	vector<string> ord;
	unsigned int index;
	in_adu->read(p);
	ord = instruction_split(p);
	index = std::stoi(ord[0]);
	ptrs[index-1]->destination = ord[1];
	if(ptrs[index-1]->qj == 0)
		ptrs[index-1]->ready = true;
	if(rob_buff[0]->entry == index && ptrs[index-1]->ready)
		rob_head_value_event.notify(1,SC_NS);
}
void reorder_buffer::check_conflict()
{
	string p;
	unsigned int rob_pos,last_st;
	vector<string> ord;
	while(true)
	{
		last_st = 0;
		in_slb->read(p);
		if(p != "F")
		{
			ord = instruction_split(p);
			rob_pos = std::stoi(ord[0]);
			for(unsigned int i = 0 ; i < rob_pos-1 ; i++)
				if(	ptrs[i]->instruction.at(0) == 'S' && ptrs[i]->busy && ptrs[i]->destination == ord[1])
					last_st = i+1;
			out_slb->write(std::to_string(last_st));
		}
		wait();
	}
}

int reorder_buffer::busy_check()
{
	for(unsigned int i = 0 ; i < tam ; i++)
		if(!ptrs[i]->busy)
			return i;
	return -1;
}
unsigned int reorder_buffer::ask_status(bool read,string reg,unsigned int pos)
{
	string res;
	if(read)
	{
		out_rb->write("R S " + reg);
		in_rb->read(res);
		return std::stoi(res);
	}
	else
		out_rb->write("W S " + reg + " " + std::to_string(pos));
	return 0;
}
float reorder_buffer::ask_value(bool read,string reg,float value)
{
	string res;
	if(read)
	{
		out_rb->write("R V " + reg);
		in_rb->read(res);
		return std::stof(res);
	}
	else
		out_rb->write("W V " + reg + ' ' + std::to_string(value));
	return 0;
}
void reorder_buffer::mem_write(unsigned int addr,float value,unsigned int rob_pos)
{
	out_mem->write("S " + std::to_string(addr) + ' ' + std::to_string(value) + ' ' + std::to_string(rob_pos));
}
void reorder_buffer::check_dependencies(unsigned int index, float value)
{
	for(unsigned int i = 0 ; i < tam ; i++)
	{
		if(ptrs[i]->busy && ptrs[i]->instruction.at(0) == 'S')
		{
			if(ptrs[i]->qj == index)
			{
				ptrs[i]->value = value;
				ptrs[i]->qj = 0;
				if(ptrs[i]->destination != "")
					ptrs[i]->ready = true;
				if(rob_buff[0]->entry == index && ptrs[i]->ready)
					rob_head_value_event.notify(1,SC_NS);
			}
		}
		else if(ptrs[i]->busy && ptrs[i]->instruction.at(0) == 'B')
		{
			if(ptrs[i]->qj == index)
			{
				ptrs[i]->vj = value;
				ptrs[i]->qj = 0;
			}
			if(ptrs[i]->qk == index)
			{
				ptrs[i]->vk = value;
				ptrs[i]->qk = 0;
			}
			if(ptrs[i]->qj == 0 && ptrs[i]->qk == 0)
				ptrs[i]->ready = true;
			if(rob_buff[0]->entry == index && ptrs[i]->ready)
				rob_head_value_event.notify(1,SC_NS);
		}
	}
}
void reorder_buffer::_flush()
{
	rob_buff.resize(0);
	for(unsigned int i = 0 ; i < tam ; i++)
	{
		ptrs[i]->busy = false;
		ptrs[i]->ready = false;
	}
}
bool reorder_buffer::branch(int optype,unsigned int rs,unsigned int rt)
{
	switch(optype)
	{
		case 0:
			if(rs == rt)
				return true;
			return false;
		case 1:
			if(rs != rt)
				return true;
			return false;
		default:
			cerr << "Erro inesperado, abortando..." << endl << flush;
			sc_stop();
			exit(1);
	}
}
bool reorder_buffer::branch(int optype,float value)
{
	switch(optype)
	{
		case 2:
			if(value > 0)
				return true;
			return false;
		case 3:
			if(value < 0)
				return true;
			return false;
		case 4:
			if(value >= 0)
				return true;
			return false;
		case 5:
			if(value <= 0)
				return true;
			return false;
		default:
			cerr << "Erro inesperado, abortando..." << endl << flush;
			sc_stop();
			exit(1);
	}
}