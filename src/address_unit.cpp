#include "address_unit.hpp"
#include "general.hpp"

address_unit::address_unit(sc_module_name name,unsigned int t, nana::listbox::cat_proxy instr_t):
sc_module(name),
delay_time(t),
instruct_table(instr_t)
{
	SC_THREAD(leitura_issue);
	sensitive << in_issue;
	dont_initialize();
	SC_THREAD(leitura_cdb);
	sensitive << in_cdb;
	dont_initialize();
	SC_THREAD(addr_issue);
	sensitive << addr_queue_event;
	dont_initialize();
}

void address_unit::leitura_issue()
{
	bool store;
	while(true)
	{
		in_issue->read(p);
		ord = instruction_split(p);
		mem_ord = offset_split(ord[2]);
		a = std::stoi(mem_ord[0]);
		instr_pos = std::stoi(ord[3]);
		rob_pos = std::stoi(ord[4]);
		regst = ask_status(true,mem_ord[1]);
		if(ord[0].at(0) == 'S')
			store = true;
		else
			store = false;
		if(regst == 0)
		{
			cout << "a" << endl << flush;
			wait(SC_ZERO_TIME);
			cout << "b" << endl << flush;
			instruct_table.at(instr_pos).text(EXEC,"X");
			a += ask_value(mem_ord[1]);
			if(store)
			{
				if(addr_queue.empty())
					addr_queue_event.notify(delay_time,SC_NS);
				addr_queue.push({store,true,regst,rob_pos,instr_pos,a});
			}
			else
			{
				offset_buff.push_back({store,true,regst,rob_pos,instr_pos,a});
				check_loads();
			}
		}
		else
		{
			offset_buff.push_back({store,false,regst,rob_pos,instr_pos,a});
			cout << "Instrucao " << ord[0] << " aguardando o resultado do ROB " << regst << endl << flush;
		}
		wait();
	}
}

void address_unit::leitura_cdb()
{
	string p_c;
	vector<string> ord_c;
	while(true)
	{
		in_cdb->read(p_c);
		ord_c = instruction_split(p_c);
		for(unsigned int i = 0 ; i < offset_buff.size() ; i++)
		{
			if(std::stoi(ord_c[0]) == offset_buff[i].regst)
			{
				offset_buff[i].a+= std::stoi(ord_c[1]);
				if(offset_buff[i].store)
				{
					wait(SC_ZERO_TIME);
					instruct_table.at(offset_buff[i].instr_pos).text(EXEC,"X");
					if(addr_queue.empty())
						addr_queue_event.notify(delay_time,SC_NS);
					addr_queue.push(offset_buff[i]);
					offset_buff.erase(offset_buff.begin() + i);
				}
				else
					offset_buff[i].addr_calc = true;
				check_loads();
			}
		}
	wait();
	}
}

void address_unit::addr_issue()
{
	addr_node fr;
	while(true)
	{
		while(addr_queue.empty())
			wait(addr_queue_event);
		fr = addr_queue.front();
		if(fr.store)
			out_rob->write(std::to_string(fr.rob_pos) + ' ' + std::to_string(fr.a));
		else
			out_slbuff->write(std::to_string(fr.rob_pos) + ' ' + std::to_string(fr.a));
		addr_queue.pop();
		wait(1,SC_NS);
	}
}

void address_unit::leitura_rob()
{
	queue<addr_node> empty;
	std::swap(addr_queue,empty);
	offset_buff.clear();
}

vector<string> address_unit::offset_split(string p)
{
	unsigned int i;
	vector<string> ord(2);
	for(i = 0 ; i < p.size() && p[i] != '(';i++)
		;
	ord[0] = p.substr(0,i);
	ord[1] = p.substr(i+1,p.size()-i-2);
	return ord;
}
float address_unit::ask_value(string reg)
{
	string res;
	out_rb->write("R V " + reg);
	in_rb->read(res);
	return std::stof(res);
}
unsigned int address_unit::ask_status(bool read,string reg,unsigned int pos)
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
void address_unit::check_loads()
{
	for(unsigned i = 0 ; i < offset_buff.size() && !offset_buff[i].store ; i++)
		if(offset_buff[i].addr_calc)
		{
			if(addr_queue.empty())
				addr_queue_event.notify(delay_time,SC_NS);
			addr_queue.push(offset_buff[i]);
			offset_buff.erase(offset_buff.begin()+i);
		}
}