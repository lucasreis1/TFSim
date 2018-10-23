#include<systemc.h>
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<deque>
#include<nana/gui.hpp>
#include<nana/gui/widgets/listbox.hpp>
#include<nana/gui/widgets/button.hpp>
#include<nana/gui/widgets/label.hpp>
#include "grid.hpp"

enum{
	BUSY = 1,
	OP = 2,
	VJ = 3,
	VK = 4,
	QJ = 5,
	QK = 6,
	A = 7
};

enum{
	ISS = 1,
	EXEC = 2,
	WRITE = 3
};

using std::string;
using std::vector;
using std::map;
using std::fstream;
using std::deque;

vector<string> instruction_split(string p)
		{
			vector<string> ord;
			unsigned int i,last_pos;
			last_pos = 0;
			for(i = 0 ; i < p.size() ; i++)
				if(p[i] == ' ' || p[i] == ',')
				{
					ord.push_back(p.substr(last_pos,i-last_pos));
					last_pos = i+1;
				}
			ord.push_back(p.substr(last_pos,p.size()-last_pos));
			return ord;
		}

class write_if_f: virtual public sc_interface
{
	public:
		virtual void write(string) = 0;
		virtual void nb_write(string) = 0;
};

class read_if_f: virtual public sc_interface
{
	public:
		virtual void read(string &) = 0;
		virtual void nb_read(string &) = 0;
		virtual void notify() = 0;
};

class write_if: virtual public sc_interface
{
	public:
		virtual void write(string) = 0;
};

class read_if: virtual public sc_interface
{
	public:
		virtual void read(string &) = 0;
};

class bus: public sc_channel, public write_if, public read_if
{
	public:
		bus(sc_module_name name): sc_channel(name){stamp = sc_time(-1,SC_NS);}
		void write(string p)
		{
			if(sc_time_stamp() == stamp)
				wait(sc_time(1,SC_NS));
			stamp = sc_time_stamp();
			palavra = p;
			write_event.notify();
		}
		const sc_event& default_event() const
		{
			return write_event;
		}
		void read(string &p)
		{
			p = palavra;
		}
	private:
		string palavra;
		sc_event write_event;
		sc_time stamp;
};

class cons_bus: public sc_channel, public write_if_f, public read_if_f
{
	public:
		cons_bus(sc_module_name name): sc_channel(name){palavra = " ";}		
		void write(string p)
		{
			if(palavra != " ")
				wait(read_event);
			palavra = p;
			write_event.notify(SC_ZERO_TIME);
			wait(read_event);
		}
		void nb_write(string p)
		{
			palavra = p;
		}
		void read(string &p)
		{
			if(palavra != " ")
			{
				p = palavra;
				read_event.notify(SC_ZERO_TIME);
				palavra = " ";
			}
			else
				wait(write_event);
		}
		void nb_read(string &p)
		{
			if(palavra != " ")
				p  = palavra;
		}
		void notify()
		{
			palavra = " ";
			read_event.notify(SC_ZERO_TIME);
		}
		const sc_event& default_event() const
		{
			return write_event;
		}
	private:
		string palavra;
		sc_event write_event;
		sc_event read_event;
};

class cons_bus_fast: public sc_channel, public write_if_f, public read_if_f
{
	public:
		cons_bus_fast(sc_module_name name): sc_channel(name){palavra = " ";}
		void write(string p)
		{
			if(palavra != " ")
				wait(read_event);
			palavra = p;
			write_event.notify();
			wait(read_event);
		}
		void nb_write(string p)
		{
			palavra = p;
		}
		void read(string &p)
		{
			if(palavra != " ")
			{
				p = palavra;
				read_event.notify();
				palavra = " ";
			}
			else
				wait(write_event);
		}
		void nb_read(string &p)
		{
			if(palavra != " ")
			{
				p = palavra;
				palavra = " ";
			}
		}
		void notify()
		{
			read_event.notify();
		}
		const sc_event& default_event() const
		{
			return write_event;
		}
	private:
		string palavra;
		sc_event write_event;
		sc_event read_event;
};


/*
	ESTRUTURA DE INSTRUÇÃO
	DADD rd, rs, rt
	LOAD rt, offset(rs)
	Rx, valor
*/

class clock1: public sc_module
{
	public:
		sc_port<write_if> out;
		SC_HAS_PROCESS(clock1);
		clock1(sc_module_name name, int dl, nana::label &clk): sc_module(name), delay(dl), clock_count(clk)
		{
			SC_THREAD(main);
		}
		void main()
		{
			while(true)
			{
				sc_pause();
				out->write("");
				clock_count.caption(sc_time_stamp().to_string());
				wait(delay,SC_NS);
			}
		}
	private:
		int delay;
		nana::label &clock_count;
};

class instruction_queue: public sc_module
{
	public:
		sc_port<read_if> in;
		sc_port<write_if_f> out;
		vector<string> instruct_queue;
		int pc;
		SC_HAS_PROCESS(instruction_queue);
		instruction_queue(sc_module_name name, vector<string> inst_q, nana::listbox &instr): sc_module(name), instruct_queue(inst_q), instructions(instr)
		{
			SC_THREAD(main);
			sensitive << in;
			dont_initialize();
		}
		void main()
		{
			auto cat = instructions.at(0);
			for(pc = 0; pc < (int)instruct_queue.size() ; pc++)
			{
				if(pc-1 > 0)
					cat.at(pc-2).select(false);
				if(pc)
				{
					cat.at(pc-1).select(true,true);
					cat.at(pc-1).text(ISS,"X");
				}
				out->write(instruct_queue[pc]);
				wait();
			}
		}
	private:
		nana::listbox &instructions;
};

/*
	ESTRUTURA DO REG_BANK (IN)
	R/W S/V REGISTER_NUMBER VALUE
	ESTRUTURA DO REG_BANK(CDB_IN)
	CDB_INDEX VALUE
	ESTRUTURA DO REG_BANK (OUT)
	VALUE
*/
class register_bank: public sc_module
{
	public:
		sc_port<read_if_f> in;
		sc_port<write_if_f> out;
		sc_port<read_if> in_cdb;
		SC_HAS_PROCESS(register_bank);
		register_bank(sc_module_name name,nana::listbox &regs): 
						sc_module(name), registers(regs)
		{
			/*reg_status_fp.resize(32);
			reg_status_fp.assign(32,0);
			reg_status_int.resize(32);
			reg_status_int.assign(32,0);*/
			SC_THREAD(le_bus);
			sensitive << in;
			dont_initialize();
			SC_METHOD(le_cdb);
			sensitive << in_cdb;
			dont_initialize();
		}
		void le_bus()
		{
			vector<string> ord;
			unsigned int index;
			string p;
			bool fp;
			auto cat = registers.at(0);
			while(true)
			{
				in->read(p);
				ord = instruction_split(p);
				index = std::stoi(ord[2].substr(1,ord[2].size()-1));
				if(ord[2].at(0) == 'F')
					fp = true;
				else
					fp = false;
				if(ord[0] == "R")
				{
					if(ord[1] == "S")
					{
						if(fp)
							out->nb_write(cat.at(index).text(FQ));
						else
							out->nb_write(cat.at(index).text(IQ));
					}
					else
					{
						if(fp)
							out->nb_write(cat.at(index).text(FVALUE));
						else
							out->nb_write(cat.at(index).text(IVALUE));
					}
				}
				else
				{
					if(ord[1] == "S")
					{
						if(fp)
							cat.at(index).text(FQ,ord[3]);
							//reg_status_fp[index] = std::stof(ord[3]);
						else
							cat.at(index).text(IQ,ord[3]);
							//reg_status_int[index] = (int)std::stof(ord[3]);
					}
					else
					{
						if(fp)
							//reg_values_fp[index] = std::stof(ord[3]);
							cat.at(index).text(FVALUE,ord[3]);
						else
							cat.at(index).text(IVALUE,ord[3]);
							//reg_values_int[index] = (int)std::stof(ord[3]);
					}
				}
				wait();
			}
		}

		void le_cdb()
		{
			//int rs_index;
			string p;
			//float value;
			vector<string> ord;
			in_cdb->read(p);
			ord = instruction_split(p);
			auto cat = registers.at(0);
			//rs_index = std::stoi(ord[0]);
			//value = std::stof(ord[1]);
			for(unsigned int i = 0 ; i < 32 ; i++)
			{
				//if(reg_status_fp[i] == rs_index)
				if(cat.at(i).text(FQ) == ord[0])
				{
					//reg_status_fp[i] = 0;
					//reg_values_fp[i] = value;
					cat.at(i).text(FQ,"0");
					cat.at(i).text(FVALUE,ord[1]);
					cout << "Valor de F" << i << " atualizado para " << ord[1] << endl << flush;
				}
				//if(reg_status_int[i] == rs_index)
				if(cat.at(i).text(IQ) == ord[0])
				{
					//reg_status_int[i] = 0;
					//reg_values_int[i] = (int)value;
					cat.at(i).text(IQ,"0");
					cat.at(i).text(IVALUE,ord[1]);
					cout << "Valor de R" << i << " atualizado para " << ord[1] << endl << flush;
				}
			}
		}

	private:
		nana::listbox &registers;
		enum
		{
			IVALUE = 1,
			IQ = 2,
			FVALUE = 4,
			FQ = 5
		};
		
};

/*
	ESTRUTURA DA REQUISIÇAO DA MEMORIA
	L/S MEM_POS DADO/RS_NUMBER
*/

class memoria: public sc_module
{
	public:
		sc_port<read_if> in;
		sc_port<write_if> out;
		SC_HAS_PROCESS(memoria);
		memoria(sc_module_name name, nana::grid &m): sc_module(name), mem(m)
		{
			SC_METHOD(leitura_bus);
			sensitive << in;
			dont_initialize();
		}
		void leitura_bus()
		{
			vector<string> ord;
			unsigned int pos;
			string escrita_saida;
			in->read(p);
			ord = instruction_split(p);
			pos = std::stoi(ord[1]);
			if(ord[0] == "L")
			{
				cout << "Instrucao terminada com resultado " << mem.Get(pos) << " para escrever na estaçao de reserva " << ord[2] << endl << flush;
				escrita_saida = ord[2] + ' ' + mem.Get(pos);
				out->write(escrita_saida);
			}
			else
			{
				mem.Set(pos,ord[2]);
			}
		}
	private:
		string p;
		nana::grid &mem;
};


class issue_control: public sc_module
{
	public:
		sc_port<read_if_f> in;
		sc_port<write_if_f> out_rsv;
		sc_port<write_if_f> out_slbuff;
		SC_HAS_PROCESS(issue_control);
		issue_control(sc_module_name name): sc_module(name)
		{
			res_type = {{"DADD",1},{"DADDI",1},{"DADDU",1},{"DADDIU",1},{"DSUB",1},{"DSUBU",1},{"DMUL",1},{"DMULU",1},{"DDIV",1},{"DDIVU",1},{"LD",2},{"SD",2}};
			SC_THREAD(issue_select);
			sensitive << in;
			dont_initialize();
		}
		void issue_select()
		{
			while(true)
			{
				in->nb_read(p);
				ord = instruction_split(p);
				switch(res_type[ord[0]])
				{
					case 1:
						out_rsv->write(p);
						break;
					case 2:
						out_slbuff->write(p);
						break;
					default:
						cerr << "Instruçao nao suportada!" << endl << flush;
						exit(1);
				}
				in->notify();
				wait();
			}
		}
	private:
		string p;
		vector<string> ord;
		map<string,unsigned short int> res_type;
};

class res_station: public sc_module
{
	public:
		int id;
		string type_name;
		bool Busy,isFirst;
		string op;
		int vj,vk,qj,qk;
		unsigned int a;
		map<string,int> instruct_time;
		sc_port<write_if> out;
		sc_port<read_if> in;
		sc_port<write_if> out_mem;
		sc_event exec_event;
		sc_event isFirst_event;
		SC_HAS_PROCESS(res_station);
		res_station(sc_module_name name,int i, string n, map<string,int> inst_map, const nana::listbox::item_proxy item):sc_module(name), id(i), type_name(n), instruct_time(inst_map), table_item(item)
		{
			Busy = isFirst = false;
			vj = vk = qj = qk = a = 0;
			SC_THREAD(exec);
			sensitive << exec_event;
			dont_initialize();
			SC_METHOD(leitura);
			sensitive << in;
			dont_initialize();
		}
		void exec()
		{
			while(true)
			{
				//Enquanto houver dependencia de valor em outra RS, espere
				while(qj || qk)
					wait(val_enc);
				float res = 0;
				if(op.substr(0,4) == "DADD")
					res = vj + vk;
				else if(op.substr(0,4) == "DSUB")
					res = vj - vk;
				else if(op.substr(0,4) == "DMUL")
					res = vj*vk;
				else if(op.substr(0,4) == "DDIV")
				{
					if(vk)
						res = vj/vk;
					else
						cout << "Divisao por 0, instrucao ignorada!" << endl;
				}
				else if(a)
				{
					a += vk;
					table_item->text(A,std::to_string(a));
					table_item->text(VK,"");
				}
				cout << "Execuçao da instruçao " << op << " iniciada no ciclo " << sc_time_stamp() << " em " << name() << endl << flush;
				wait(sc_time(instruct_time[op],SC_NS));
				if(!a)
				{
					string escrita_saida = std::to_string(id) + ' ' + std::to_string(res);
					cout << "Instrucao " << op << " completada no ciclo " << sc_time_stamp() << " em " << name() << " com resultado " << res << endl << flush;
					out->write(escrita_saida);
				}
				else
				{
					if(!isFirst)
						wait(isFirst_event);
					if(op.at(0) == 'L')
						mem_req(true,a,id);
					else
					{
						mem_req(false,a,vj);
						cout << "Instrucao " << op << " completada no ciclo " << sc_time_stamp() << " em " << name() << " gravando na posicao de memoria " << a << " o resultado " << vj << endl << flush;
					}
					isFirst = false;
					a = 0;
				}
				Busy = false;
				cout << "estacao " << id << " liberada no ciclo " << sc_time_stamp() << endl << flush;
				clean_item(); //Limpa a tabela na interface grafica
				wait();
			}
		}
		void leitura()
		{
			if(qj || qk)
			{
				unsigned int i;
				int rs_source;
				string value;
				in->read(p);
				for(i = 0 ; i < p.size() && p[i] != ' '; i++)
					;
				rs_source = std::stoi(p.substr(0,i));
				if(qj == rs_source)
				{
					qj = 0;
					value = p.substr(i+1,p.size() - i - 1);
					vj = std::stoi(value);
					table_item->text(VJ,value);
					table_item->text(QJ,"");
					cout << "Instrucao " << op << " conseguiu o valor " << vj << " da RS_" << rs_source << endl << flush; 
					val_enc.notify(1,SC_NS);
				}
				if(qk == rs_source)
				{
					qk = 0;
					value = p.substr(i+1,p.size() - i - 1);
					vk = std::stoi(value);
					table_item->text(VK,value);
					table_item->text(QK,"");
					cout << "Instrucao " << op << " conseguiu o valor " << vk << " da RS_" << rs_source << endl << flush; 
					val_enc.notify(1,SC_NS);
				}
			}
		}
		void clean_item()
		{
			for(unsigned i = 2 ; i < table_item->columns(); i++)
				table_item->text(i,"");
			table_item->text(BUSY,"False");
		}
	private:
		string p;
		sc_event val_enc;
		nana::listbox::item_proxy table_item;

		void mem_req(bool load,unsigned int addr,int value)
		{
			string escrita_saida;
			string temp = std::to_string(addr) + ' ' + std::to_string(value);
			if(load)
				escrita_saida = "L " + temp;
			else
				escrita_saida = "S " + temp;
			out_mem->write(escrita_saida);
		}
};

class res_vector: public sc_module
{
	public:
		vector<res_station *> rs;
		sc_port<read_if_f> in_issue;
		sc_port<read_if> in_cdb;
		sc_port<write_if> out_cdb;
		sc_port<read_if_f> in_rb;
		sc_port<write_if_f> out_rb;
		sc_port<write_if> out_mem;
		SC_HAS_PROCESS(res_vector);
		res_vector(sc_module_name name,unsigned int t1, unsigned int t2,map<string,int> instruct_time, nana::listbox &lsbox):sc_module(name), table(lsbox)
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
				rs[i] = new res_station(texto.c_str(),i+1,texto,instruct_time,cat.at(i));
				rs[i]->in(in_cdb);
				rs[i]->out(out_cdb);
				rs[i]->out_mem(out_mem);
			}
			SC_THREAD(leitura_issue);
			sensitive << in_issue;
			dont_initialize();
		}
		void leitura_issue()
		{
			string p;
			vector<string> ord;
			int pos,regst;
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
				rs[pos]->op = ord[0];
				cat.at(pos).text(OP,ord[0]);
				ask_status(false,ord[1],pos+1);
				regst = ask_status(true,ord[2]);
				if(regst == 0)
				{
					value = ask_value(ord[2]);
					rs[pos]->vj = value;
					cat.at(pos).text(VJ,std::to_string(value));
				}
				else
				{
					cout << "instruçao " << ord[0] << " aguardando reg R" << ord[2] << endl << flush;
					rs[pos]->qj = regst;
					cat.at(pos).text(QJ,std::to_string(regst));
				}
				regst = ask_status(true,ord[3]);
				if(ord[0].at(ord[0].size()-1) == 'I')
				{
					rs[pos]->vk = std::stoi(ord[3]);
					cat.at(pos).text(VK,ord[3]);
				}
				else if(regst == 0)
				{
					value = ask_value(ord[3]);
					rs[pos]->vk = value;
					cat.at(pos).text(VK,std::to_string(value));
				}
				else
				{
					cout << "instruçao " << ord[0] << " aguardando reg R" << ord[3] << endl << flush;
					rs[pos]->qk = regst;
					cat.at(pos).text(QK,std::to_string(regst));
				}
				rs[pos]->Busy = true;
				cat.at(pos).text(BUSY,"True");
				rs[pos]->exec_event.notify(1,SC_NS);
				wait();
			}
		}

	private:
		map<string,int> res_type;
		unsigned int tam_pos[3];
		nana::listbox &table;

		int busy_check(string inst)
		{
			unsigned int inst_type = res_type[inst];
			for(unsigned int i = tam_pos[inst_type] ; i < tam_pos[inst_type + 1] ; i++)
				if(!rs[i]->Busy)
					return i;
			return -1;
		}
		float ask_value(string reg)
		{
			string res;
			out_rb->write("R V " + reg);
			in_rb->read(res);
			return std::stof(res);
		}
		unsigned int ask_status(bool read,string reg,unsigned int pos = 0)
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
};

class sl_buffer: public sc_module
{
	public:
		deque<res_station *> sl_buff;
		sc_port<read_if_f> in_issue;
		sc_port<read_if_f> in_rb;
		sc_port<write_if_f> out_rb;
		sc_port<write_if> out_mem;
		sc_port<read_if> in_cdb;
		sc_port<write_if> out_cdb;
		SC_HAS_PROCESS(sl_buffer);

		sl_buffer(sc_module_name name,unsigned int t,unsigned int t_outros,map<string,int> instruct_time, nana::listbox &lsbox): sc_module(name), tam(t), tam_outros(t_outros), table(lsbox)
		{
			string texto;
			ptrs = new res_station*[tam];
			auto cat = table.at(0);
			for(unsigned int i = 0 ; i < tam ; i++)
			{
				texto = "Load" + std::to_string(i+1);
				cat.append(texto);
				ptrs[i] = new res_station(texto.c_str(),i+t_outros,texto,instruct_time,cat.at(i+t_outros));
				ptrs[i]->in(in_cdb);
				ptrs[i]->out(out_cdb);
				ptrs[i]->out_mem(out_mem);
			}
			SC_THREAD(leitura_issue);
			sensitive << in_issue;
			dont_initialize();
			SC_METHOD(sl_buff_control);
			sensitive << out_mem;
			dont_initialize();
		}
		void leitura_issue()
		{
			string p;
			vector<string> ord,mem_ord;
			int pos;
			int regst;
			auto cat = table.at(0);
			while(true)
			{
				in_issue->nb_read(p);
				ord = instruction_split(p);
				pos = busy_check();
				while(pos == -1)
				{
					cout << "Todas as estacoes ocupadas para a instrucao " << p << " no ciclo " << sc_time_stamp() << endl << flush;
					wait(out_mem->default_event());
					pos = busy_check();
					if(pos != -1)
						wait(1,SC_NS);
				}
				in_issue->notify();
				cout << "Issue da instrução " << ord[0] << " no ciclo " << sc_time_stamp() << " para " << ptrs[pos]->type_name << endl << flush;
				ptrs[pos]->op = ord[0];
				cat.at(pos+tam_outros).text(OP,ord[0]);
				mem_ord = offset_split(ord[2]);
				if(ord[0].at(0) == 'L')
					ask_status(false,ord[1],ptrs[pos]->id);
				else
				{
					regst = ask_status(true,ord[1]);
					if(regst == 0)
					{
						float value = ask_value(ord[1]);
						ptrs[pos]->vj = value;
						cat.at(pos+tam_outros).text(VJ,std::to_string(value));
					}
					else
					{
						cout << "instruçao " << ord[0] << " aguardando reg R" << ord[1] << endl << flush;
						ptrs[pos]->qj = regst;
						cat.at(pos+tam_outros).text(QJ,std::to_string(regst+1));
					}	
				}
				regst = ask_status(true,mem_ord[1]);
				if(regst == 0)
				{
					float value = ask_value(mem_ord[1]);
					ptrs[pos]->vk = value;
					cat.at(pos+tam_outros).text(VK,std::to_string(value));
				}
				else
				{
					cout << "instruçao " << ord[0] << " aguardando reg " << mem_ord[1] << endl << flush;
					ptrs[pos]->qk = regst;
					cat.at(pos+tam_outros).text(QK,std::to_string(regst));
				}
				ptrs[pos]->a = std::stoi(mem_ord[0]);
				ptrs[pos]->Busy = true;
				cat.at(pos+tam_outros).text(A,mem_ord[0]);
				cat.at(pos+tam_outros).text(BUSY,"True");
				if(sl_buff.empty())
					ptrs[pos]->isFirst = true;
				sl_buff.push_back(ptrs[pos]);
				ptrs[pos]->exec_event.notify(1,SC_NS);
				wait();
			}
		}
		void sl_buff_control()
		{
			sl_buff.pop_front();
			if(!sl_buff.empty())
			{
				sl_buff[0]->isFirst = true;
				sl_buff[0]->isFirst_event.notify(1,SC_NS);
			}
		}

	private:
		unsigned int tam;
		unsigned int tam_outros;
		res_station **ptrs;
		nana::listbox &table;
		int busy_check()
		{
			for(unsigned int i = 0 ; i < tam ; i++)
				if(ptrs[i]->Busy == false)
					return i;
			return -1;
		}

		vector<string> offset_split(string p)
		{
			unsigned int i;
			vector<string> ord(2);
			for(i = 0 ; i < p.size() && p[i] != '(';i++)
				;
			ord[0] = p.substr(0,i);
			ord[1] = p.substr(i+1,p.size()-i-2);
			return ord;
		}

		unsigned int ask_status(bool read,string reg,unsigned int pos = 0)
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
		float ask_value(string reg)
		{
			string res;
			out_rb->write("R V " + reg);
			in_rb->read(res);
			return std::stof(res);
		}
};

class top: public sc_module
{
	public:
		bus *CDB,*mem_bus, *clock_bus;
		cons_bus *inst_bus;
		cons_bus_fast *rb_bus;
		cons_bus *rst_bus;
		cons_bus *sl_bus;
		issue_control *iss_ctrl;
		clock1 *clk;
		res_vector *rst;
		sl_buffer *slb;
		register_bank *rb;
		memoria *mem;
		instruction_queue *fila;
		
	top(sc_module_name name,unsigned int t1, unsigned int t2,unsigned int t3,map<string,int> instruct_time,
		vector<string> instruct_queue, nana::listbox &table, nana::grid &mem_gui, nana::listbox &regs, nana::listbox &instr, nana::label &ccount): sc_module(name)
	{
		CDB = new bus("CDB");
		mem_bus = new bus("mem_bus");
		clock_bus = new bus("clock_bus");
		inst_bus = new cons_bus("inst_bus");
		rb_bus = new cons_bus_fast("rb_bus");
		rst_bus = new cons_bus("rst_bus");
		sl_bus = new cons_bus("sl_bus");
		iss_ctrl = new issue_control("issue_control");
		clk = new clock1("clock",1,ccount);
		fila = new instruction_queue("fila_inst",instruct_queue,instr);
		rst = new res_vector("rs_vc",t1,t2,instruct_time,table);
		rb = new register_bank("register_bank", regs);
		slb = new sl_buffer("sl_buffer",t3,t1+t2,instruct_time,table);
		mem = new memoria("memoria", mem_gui);
		//fila->clock(clock);
		clk->out(*clock_bus);
		fila->in(*clock_bus);
		fila->out(*inst_bus);
		iss_ctrl->in(*inst_bus);
		iss_ctrl->out_rsv(*rst_bus);
		iss_ctrl->out_slbuff(*sl_bus);
		rst->in_issue(*rst_bus);
		rst->in_cdb(*CDB);
		rst->out_cdb(*CDB);
		rst->in_rb(*rb_bus);
		rst->out_rb(*rb_bus);
		rst->out_mem(*mem_bus);
		slb->in_issue(*sl_bus);
		slb->in_cdb(*CDB);
		slb->out_cdb(*CDB);
		slb->in_rb(*rb_bus);
		slb->out_rb(*rb_bus);
		slb->out_mem(*mem_bus);
		rb->in(*rb_bus);
		rb->out(*rb_bus);		
		rb->in_cdb(*CDB);
		mem->in(*mem_bus);
		mem->out(*CDB);
	}
};

int start(int argc, char *argv[], nana::listbox &table, vector<string> &instruction_queue, nana::grid &mem_gui, nana::listbox &regs, nana::listbox &instr, nana::label &clk)
{
	map<string,int> instruct_time{{"DADD",4},{"DADDI",4},{"DSUB",6},{"DSUBI",6},{"DMUL",10},{"DMULI",10},{"DDIV",16},{"DDIVI",16},{"SD",1},{"LD",2}};
	top top1("top",3,2,2,instruct_time,instruction_queue,table,mem_gui,regs,instr,clk);
	sc_start();
	return 0;
}

int sc_main(int argc, char *argv[])
{
	using namespace nana;
	std::vector<std::string> columns = {"Name","Busy","Op","Vj","Vk","Qj","Qk","A"}; 
	std::vector<int> sizes;
	form fm(API::make_center(1000,600));
	place plc(fm);
	place upper(fm);
	place lower(fm);
	plc.div("<vert<weight = 50% <vert weight = 50% <weight = 70% rst> <instr> ><vert <weight = 50% memor> < <regs> <weight=20%>> > > <weight = 5%> < <gap = 10 btns><weight = 80%> > <clk_c> <weight=30%> >");
	listbox table(fm);
	listbox reg(fm);
	listbox instruct(fm);
	button botao(fm);
	button clock_control(fm);
	label clock_count(fm);
	grid memory(fm,rectangle(),10,50);
	botao.caption("START");
	clock_control.caption("NEXT CYCLE");
	plc["rst"] << table;
	plc["btns"] << botao << clock_control;
	plc["memor"] << memory;
	plc["regs"] << reg;
	plc["instr"] << instruct;
	plc["clk_c"] << clock_count;
	plc.collocate();
	//instruct.show_header(false);
	instruct.scheme().item_selected = colors::red;

	for(unsigned int i = 0 ; i < columns.size() ; i++)
	{
		table.append_header(columns[i].c_str());
		table.column_at(i).width(60);
	}
	columns = {"","Value","Qi"};
	for(unsigned int k = 0 ; k < 2 ; k++)
		for(unsigned int i = 0 ; i < columns.size() ; i++)
			reg.append_header(columns[i].c_str());

	for(unsigned int i = 0 ; i < reg.column_size() ; i++)
		reg.column_at(i).width(60);
	
	auto cat = reg.at(0);
	for(int i = 0 ; i < 32 ;i++)
	{
		string index = std::to_string(i);
		cat.append("R" + index);
		cat.at(i).text(3,"F" + index);
	}

	columns = {"Instruction","Issue","Execute","Write Result"};
	sizes = {150,60,70,100};
	for(unsigned int i = 0 ; i < columns.size() ; i++)
	{
		instruct.append_header(columns[i]);
		instruct.column_at(i).width(sizes[i]);
	}

	ifstream inFile;
	vector<string> instruction_queue;
	string line;
	int value,i = 0;
	float value_fp;
	if(argc < 4)
	{
		cout << "Uso: ./ex.x <lista_instrucoes> <reg_int_valores_iniciais> <reg_fp_valores_iniciais> <memoria_valores_iniciais>" << endl;
		return 1;
	}
	inFile.open(argv[1]);
	if(!inFile.is_open())
	{
		cerr << "Arquivo " << argv[1] << " nao existe!" << endl;
		return 1;
	}
	auto c = instruct.at(0);
	cout << "---------------------------------------------------" << endl;
	cout << "\t\tFILA DE INSTRUCOES" << endl;
	while(getline(inFile,line))
	{
		cout << line << endl;
		instruction_queue.push_back(line);
		c.append(line);
	}
	cout << "---------------------------------------------------" << endl << endl;
	inFile.close();
	inFile.open(argv[2]);
	if(!inFile.is_open())
	{
		cerr << "Arquivo " << argv[2] << " nao existe!" << endl;
		return 1;
	}
	i = 0;
	while(inFile >> value)
	{
		cat.at(i).text(1,std::to_string(value));
		cat.at(i).text(2,"0");
		i++;
	}
	inFile.close();

	i = 0;
	inFile.open(argv[3]);
	if(!inFile.is_open())
	{
		cerr << "Arquivo " << argv[3] << " nao existe!" << endl;
		return 1;
	}
	while(inFile >> value_fp)
	{
		cat.at(i).text(4,std::to_string(value_fp));
		cat.at(i).text(5,"0");
		i++;
	}
	inFile.close();
	inFile.open(argv[4]);
	if(!inFile.is_open())
	{
		cerr << "Arquivo " << argv[4] << " nao existe!" << endl;
		return 1;
	}
	while(inFile >> value)
		memory.Push(std::to_string(value));
	for(unsigned int i = 0 ; i < instruct.column_size() ; i++)
		
	clock_control.enabled(false);

	botao.events().click([&]
											{
												botao.enabled(false);
												clock_control.enabled(true);
												start(argc,argv,table,instruction_queue,memory,reg,instruct,clock_count);
											});
	clock_control.events().click([]
													{
														if(sc_is_running())
															sc_start();
													});
	fm.show();
	exec();
	return 0;
}