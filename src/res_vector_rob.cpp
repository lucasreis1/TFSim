#include "res_vector_rob.hpp"
#include "general.hpp"

res_vector_rob::res_vector_rob(sc_module_name name,unsigned int t1, unsigned int t2,map<string,int> instruct_time, nana::listbox &lsbox, nana::listbox::cat_proxy ct):
sc_module(name),
table(lsbox)
{
	res_type = {{"DADD",0},{"DADDI",0},{"DADDU",0},{"DADDIU",0},{"DSUB",0},{"DSUBU",0},{"DMUL",1},{"DMULU",1},{"DDIV",1},{"DDIVU",1}};
	auto cat = table.at(0);
	string texto;
	rs.resize(t1+t2);
	tam_pos[0] = 0;
	tam_pos[1] = t1;
	tam_pos[2] = t1+t2;
	for(unsigned int i = 0 ; i < t1+t2 ; i++)
	{
		if(i < t1)
			texto = "Add" + std::to_string(i+1);
		else
			texto = "Mult" + std::to_string(i-t1+1);
		cat.append(texto);
		cat.at(cat.columns()-1).text(BUSY,"False");
		rs[i] = new res_station(texto.c_str(),i+1,texto,instruct_time,cat.at(i),ct);
		rs[i]->in(in_cdb);
		rs[i]->out(out_cdb);
		rs[i]->out_mem(out_mem);
	}
	SC_THREAD(leitura_issue);
	sensitive << in_issue;
	dont_initialize();
	SC_METHOD(leitura_rob);
	sensitive << in_rob;
	dont_initialize();
}

res_vector_rob::~res_vector_rob()
{
	for(unsigned int i = 0 ; i < rs.size() ; i++)
		delete rs[i];
}

void res_vector_rob::leitura_issue()
{
	string p;
	vector<string> ord;
	int pos,rob_pos;
	int regst;
	float value;
	auto cat = table.at(0);
	while(true)
	{
		in_issue->nb_read(p);
		ord = instruction_split(p);
		pos = busy_check(ord[0]);
		while(pos == -1)
		{
			cout << "Todas as estacoes ocupadas para a instrucao " << p << endl << flush;
			wait(in_cdb->default_event());
			wait(1,SC_NS);
			pos = busy_check(ord[0]);
		}
		in_issue->notify();
		cout << "Issue da instrução " << ord[0] << " no ciclo " << sc_time_stamp() << " para " << rs[pos]->type_name << endl << flush;
		rob_pos = std::stoi(ord[4]);
		rs[pos]->op = ord[0];
		rs[pos]->dest = rob_pos;
		rs[pos]->instr_pos = std::stoi(ord[5]);
		cat.at(pos).text(OP,ord[0]);
		regst = ask_status(ord[2]);
		if(regst == 0)
		{
			value = ask_value(ord[2]);
			rs[pos]->vj = value;
			if(ord[2].at(0) == 'R')
				cat.at(pos).text(VJ,std::to_string((int)value));
			else
				cat.at(pos).text(VJ,std::to_string(value));
		}
		else
		{
			cout << "instruçao " << ord[0] << " aguardando reg " << ord[2] << endl << flush;
			rs[pos]->qj = regst;
			cat.at(pos).text(QJ,std::to_string(regst));
		}
		regst = ask_status(ord[3]);
		if(ord[0].at(ord[0].size()-1) == 'I')
		{
			value = std::stof(ord[3]);
			rs[pos]->vk = value;
			cat.at(pos).text(VK,std::to_string((int)value));
		}
		else if(regst == 0)
		{
			value = ask_value(ord[3]);
			rs[pos]->vk = value;
			if(ord[3].at(0) == 'R')
				cat.at(pos).text(VK,std::to_string((int)value));
			else
				cat.at(pos).text(VK,std::to_string(value));
		}
		else
		{
			cout << "instruçao " << ord[0] << " aguardando reg " << ord[2] << endl << flush;
			rs[pos]->qk = regst;
			cat.at(pos).text(QK,std::to_string(regst));
		}
		rs[pos]->Busy = true;
		cat.at(pos).text(BUSY,"True");
		rs[pos]->exec_event.notify(1,SC_NS);
		wait();
	}
}

void res_vector_rob::leitura_rob()
{
	for(unsigned int i = 0 ; i < rs.size() ; i++)
	{
		if(rs[i]->Busy)
		{
			rs[i]->isFlushed = true;
			rs[i]->qj = rs[i]->qk = 0;
			cat.at(i).text(QJ,"");
			cat.at(i).text(QK,"");
			rs[i]->isFlushed_event.notify();
		}
	}
}

int res_vector_rob::busy_check(string inst)
{
	unsigned int inst_type = res_type[inst];
	for(unsigned int i = tam_pos[inst_type] ; i < tam_pos[inst_type + 1] ; i++)
		if(!rs[i]->Busy)
			return i;
	return -1;
}
float res_vector_rob::ask_value(string reg)
{
	string res;
	out_rb->write("R V " + reg);
	in_rb->read(res);
	return std::stof(res);
}
unsigned int res_vector_rob::ask_status(string reg)
{
	string res;
	out_rb->write("R S " + reg);
	in_rb->read(res);
	return std::stoi(res);
}