#include<systemc.h>
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<deque>

#define COMMIT 4
#define WRITE 3
#define EXECUTE 2
#define ISSUE 1

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

class write_if_fila: virtual public sc_interface
{
	public:
		virtual void write(string) = 0;
		virtual void nb_write(string) = 0;
};

class read_if_fila: virtual public sc_interface
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

class cons_bus: public sc_channel, public write_if_fila, public read_if_fila
{
	public:
		cons_bus(sc_module_name name): sc_channel(name){}		
		void write(string p)
		{
			palavra = p;
			write_event.notify(SC_ZERO_TIME);
			wait(read_event);
			//cout << "liberei em " << sc_time_stamp()  << " em " << name() << endl << flush;
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
			{
				p  = palavra;
				palavra = " ";
			}
		}
		void notify()
		{
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

/*
	ESTRUTURA DE INSTRUÇÃO
	DADD rd, rs, rt
	LOAD rt, offset(rs)
	Rx, valor
*/

class instruction_queue: public sc_module
{
	public:
		sc_in_clk clock;
		sc_port<write_if_fila> out;
		vector<string> instruct_queue;
		unsigned int pc;
		SC_HAS_PROCESS(instruction_queue);
		instruction_queue(sc_module_name name, vector<string> inst_q): sc_module(name), instruct_queue(inst_q)
		{
			SC_THREAD(main);
			sensitive << clock.pos();
			dont_initialize();
		}
		void main()
		{
			for(pc = 0; pc < instruct_queue.size() ; pc++)
			{
				out->write(instruct_queue[pc]);
				wait();
			}
		}
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
		sc_port<read_if_fila> in;
		sc_port<write_if_fila> out;
		sc_port<read_if> in_cdb;
		SC_HAS_PROCESS(register_bank);
		register_bank(sc_module_name name,vector<int> rg_values): sc_module(name), reg_values(rg_values)
		{
			reg_status.resize(32);
			reg_status.assign(32,0);
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
			while(true)
			{
				in->read(p);
				ord = instruction_split(p);
				index = std::stoi(ord[2]);
				if(ord[0] == "R")
				{
					if(ord[1] == "S")
						out->nb_write(std::to_string(reg_status[index]));
					else
						out->nb_write(std::to_string(reg_values[index]));
				}
				else
				{
					if(ord[1] == "S")
						reg_status[index] = std::stof(ord[3]);
					else
						reg_values[index] = std::stof(ord[3]);
				}
				wait();
			}
		}

		void le_cdb()
		{
			int rs_index;
			string p;
			float value;
			vector<string> ord;
			in_cdb->read(p);
			ord = instruction_split(p);
			rs_index = std::stoi(ord[0]);
			value = std::stof(ord[1]);
			for(unsigned int i = 0 ; i < reg_status.size() ; i++)
				if(reg_status[i] == rs_index)
				{
					reg_status[i] = 0;
					reg_values[i] = value;
					cout << "Valor de R" << i << " atualizado para " << value << endl << flush;
				}
		}

	private:
		vector<int> reg_status;
		vector<int> reg_values;
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
		vector<int> *mem;
		SC_HAS_PROCESS(memoria);
		memoria(sc_module_name name, vector<int> *m): sc_module(name), mem(m)
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
				escrita_saida = ord[2] + ' ' + std::to_string(mem->at(pos));
				cout << "Instrucao completa no ciclo " << sc_time_stamp() << " buscando do endereco de memoria " << pos << " o resultado " << mem->at(pos) << endl << flush;
				out->write(escrita_saida);
			}
			else
				mem->at(pos) = std::stoi(ord[2]);
		}
	private:
		string p;
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
		res_station(sc_module_name name,int i, string n, map<string,int> inst_map):sc_module(name), id(i), type_name(n), instruct_time(inst_map)
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
					a += vk;	
				cout << "Execuçao da instruçao " << op << " iniciada no ciclo " << sc_time_stamp() << " em " << name() << endl << flush;
				wait(sc_time(instruct_time[op],SC_NS));
				if(!a)
				{
					string escrita_saida = std::to_string(id) + ' ' + std::to_string(res);
					cout << "Instrucao " << op << " completada no ciclo " << sc_time_stamp() << " em " << name() << " com resultado " << res << endl << flush;
					out->write(escrita_saida);
					Busy = false;
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
					Busy = false;
					isFirst = false;
					a = 0;
				}
				wait();
			}
		}
		void leitura()
		{
			if(qj || qk)
			{
				unsigned int i;
				int rs_source;
				in->read(p);
				for(i = 0 ; i < p.size() && p[i] != ' '; i++)
					;
				rs_source = std::stoi(p.substr(0,i));
				if(qj == rs_source)
				{
					qj = 0;
					vj = std::stoi(p.substr(i+1,p.size() - i - 1));
					cout << "Instrucao " << op << " conseguiu o valor " << vj << " da RS_" << rs_source << endl << flush; 
					val_enc.notify(1,SC_NS);
				}
				if(qk == rs_source)
				{
					qk = 0;
					vk = std::stoi(p.substr(i+1,p.size() - i - 1));
					cout << "Instrucao " << op << " conseguiu o valor " << vk << " da RS_" << rs_source << endl << flush; 
					val_enc.notify(1,SC_NS);
				}
			}
		}
	private:
		string p;
		sc_event val_enc;
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

class issue_control: public sc_module
{
	public:
		sc_port<read_if_fila> in;
		sc_port<write_if_fila> out_rsv;
		sc_port<write_if_fila> out_slbuff;
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
				//cout << "Passei por aqui no ciclo " << sc_time_stamp() << endl << flush;
				switch(res_type[ord[0]])
				{
					case 1:
						//cout << "E por aqui no ciclo " << sc_time_stamp() << endl << flush;
						out_rsv->write(p);
						//cout << "E dps no ciclo " << sc_time_stamp() << endl << flush;
						break;
					case 2:
						//cout << "E por aqui no ciclo " << sc_time_stamp() << endl << flush;
						out_slbuff->write(p);
						//cout << "E dps no ciclo " << sc_time_stamp() << endl << flush;
						break;
					default:
						cerr << "Instruçao nao suportada!" << endl << flush;
						exit(1);
				}
				//cout << "E entao no ciclo " << sc_time_stamp() << endl << flush;
				in->notify();
				//cout << "Dai so faltou aqui no ciclo " << sc_time_stamp() << endl << flush;
				wait();
			}
		}
	private:
		string p;
		vector<string> ord;
		map<string,unsigned short int> res_type;
};

class res_vector: public sc_module
{
	public:
		vector<res_station *> rs;
		sc_port<read_if_fila> in_issue;
		sc_port<read_if> in_cdb;
		sc_port<write_if> out_cdb;
		sc_port<read_if_fila> in_rb;
		sc_port<write_if_fila> out_rb;
		sc_port<write_if> out_mem;
		SC_HAS_PROCESS(res_vector);
		res_vector(sc_module_name name,unsigned int t1, unsigned int t2,map<string,int> instruct_time):sc_module(name)
		{
			res_type = {{"DADD",0},{"DADDI",0},{"DADDU",0},{"DADDIU",0},{"DSUB",0},{"DSUBU",0},{"DMUL",1},{"DMULU",1},{"DDIV",1},{"DDIVU",1}};
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
				rs[i] = new res_station(texto.c_str(),i+1,texto,instruct_time);
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
			int pos,regst_i,rg1_i,rg2_i,regst;
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
				regst_i = std::stoi(ord[1].substr(1,ord[1].size()-1));
				rg1_i = std::stoi(ord[2].substr(1,ord[2].size()-1));
				rg2_i = std::stoi(ord[3].substr(1,ord[3].size()-1));
				ask_status(false,regst_i,pos+1);
				regst = ask_status(true,rg1_i);
				if(regst == 0)
					rs[pos]->vj = ask_value(rg1_i);
				else
				{
					cout << "instruçao " << ord[0] << " aguardando reg R" << rg1_i << endl << flush;
					rs[pos]->qj = regst;
				}
				regst = ask_status(true,rg2_i);
				if(ord[0].at(ord[0].size()-1) == 'I')
					rs[pos]->vk = std::stoi(ord[3]);
				else if(regst == 0)
					rs[pos]->vk = ask_value(rg2_i);
				else
				{
					cout << "instruçao " << ord[0] << " aguardando reg R" << rg1_i << endl << flush;
					rs[pos]->qk = regst;
				}
				rs[pos]->Busy = true;
				rs[pos]->exec_event.notify(1,SC_NS);
				wait();
			}
		}

	private:
		map<string,int> res_type;
		unsigned int tam_pos[3];

		int busy_check(string inst)
		{
			unsigned int inst_type = res_type[inst];
			for(unsigned int i = tam_pos[inst_type] ; i < tam_pos[inst_type + 1] ; i++)
				if(!rs[i]->Busy)
					return i;
			return -1;
		}
		float ask_value(unsigned int index)
		{
			string res;
			out_rb->write("R V " + std::to_string(index));
			in_rb->read(res);
			return std::stof(res);
		}
		unsigned int ask_status(bool read,unsigned int index,unsigned int pos = 0)
		{
			string res;
			if(read)
			{
				out_rb->write("R S " + std::to_string(index));
				in_rb->read(res);
				return std::stoi(res);
			}
			else
				out_rb->write("W S " + std::to_string(index) + " " + std::to_string(pos));
			return 0;
		}

};

class sl_buffer: public sc_module
{
	public:
		deque<res_station *> sl_buff;
		sc_port<read_if_fila> in_issue;
		sc_port<read_if_fila> in_rb;
		sc_port<write_if_fila> out_rb;
		sc_port<write_if> out_mem;
		sc_port<read_if> in_cdb;
		sc_port<write_if> out_cdb;
		SC_HAS_PROCESS(sl_buffer);

		sl_buffer(sc_module_name name,unsigned int t,unsigned int t_outros,map<string,int> instruct_time): sc_module(name), tam(t)
		{
			string texto;
			ptrs = new res_station*[tam];
			for(unsigned int i = 0 ; i < tam ; i++)
			{
				texto = "Load" + std::to_string(i);
				ptrs[i] = new res_station(texto.c_str(),i+t_outros,texto,instruct_time);
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
			int regst,regst_i,rg1_i,rg2_i;
			while(true)
			{
				in_issue->nb_read(p);
				ord = instruction_split(p);
				pos = busy_check();
				while(pos == -1)
				{
					cout << "Todas as estacoes ocupadas para a instrucao " << p << " no ciclo " << sc_time_stamp() << endl << flush;
					wait(out_mem->default_event());
					wait(1,SC_NS);
					pos = busy_check();
				}
				in_issue->notify();
				cout << "Issue da instrução " << ord[0] << " no ciclo " << sc_time_stamp() << " para " << ptrs[pos]->type_name << endl << flush;
				ptrs[pos]->op = ord[0];
				mem_ord = offset_split(ord[2]);
				if(ord[0].at(0) == 'L')
				{
					regst_i = std::stoi(ord[1].substr(1,ord[1].size()-1));
					ask_status(false,regst_i,ptrs[pos]->id);
				}
				else
				{
					rg1_i = std::stoi(ord[1].substr(1,ord[1].size()-1));
					regst = ask_status(true,rg1_i);
					if(regst == 0)
						ptrs[pos]->vj = ask_value(rg1_i);
					else
					{
						cout << "instruçao " << ord[0] << " aguardando reg R" << rg1_i << endl << flush;
						ptrs[pos]->qj = regst;
					}	
				}
				rg2_i = std::stoi(mem_ord[1].substr(1,mem_ord[1].size()-1));
				regst = ask_status(true,rg2_i);
				if(regst == 0)
					ptrs[pos]->vk = ask_value(rg2_i);
				else
				{
					cout << "instruçao " << ord[0] << " aguardando reg R" << rg2_i << endl << flush;
					ptrs[pos]->qk = regst;
				}
				ptrs[pos]->a = std::stoi(mem_ord[0]);
				ptrs[pos]->Busy = true;
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
		res_station **ptrs;
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

		float ask_value(unsigned int index)
		{
			string res;
			out_rb->write("R V " + std::to_string(index));
			in_rb->read(res);
			return std::stof(res);
		}
		unsigned int ask_status(bool read,unsigned int index,unsigned int pos = 0)
		{
			string res;
			if(read)
			{
				out_rb->write("R S " + std::to_string(index));
				in_rb->read(res);
				return std::stoi(res);
			}
			else
				out_rb->write("W S " + std::to_string(index) + " " + std::to_string(pos));
			return 0;
		}
};

class top: public sc_module
{
	public:
		sc_in_clk clock;
		bus *CDB,*mem_bus;
		cons_bus *inst_bus;
		cons_bus *rb_bus;
		cons_bus *rst_bus;
		cons_bus *sl_bus;
		issue_control *iss_ctrl;
		res_vector *rst;
		sl_buffer *slb;
		register_bank *rb;
		memoria *mem;
		instruction_queue *fila;
		
	top(sc_module_name name,unsigned int t1, unsigned int t2,unsigned int t3,map<string,int> instruct_time,
		vector<string> instruct_queue, vector<int> reg_status,vector<int> *mem_vector): sc_module(name)
	{
		CDB = new bus("CDB");
		mem_bus = new bus("mem_bus");
		inst_bus = new cons_bus("inst_bus");
		rb_bus = new cons_bus("rb_bus");
		rst_bus = new cons_bus("rst_bus");
		sl_bus = new cons_bus("sl_bus");
		iss_ctrl = new issue_control("issue_control");
		fila = new instruction_queue("fila_inst",instruct_queue);
		rst = new res_vector("rs_vc",t1,t2,instruct_time);
		rb = new register_bank("register_bank",reg_status);
		slb = new sl_buffer("sl_buffer",t3,t1+t2,instruct_time);
		mem = new memoria("memoria",mem_vector);
		fila->clock(clock);
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

int sc_main(int argc, char *argv[])
{
	map<string,int> instruct_time{{"DADD",4},{"DADDI",4},{"DSUB",6},{"DSUBI",6},{"DMUL",10},{"DMULI",10},{"DDIV",16},{"DDIVI",16},{"SD",1},{"LD",2}};
	sc_clock clock("clk");
	ifstream inFile;
	vector<string> instruction_queue;
	string line;
	vector<int> status,memoria;
	int value,i = 0;
	if(argc < 4)
	{
		cout << "Uso: ./ex.x <lista_instrucoes> <reg_valores_iniciais> <memoria_valores_iniciais>" << endl;
		return 1;
	}
	inFile.open(argv[1]);
	if(!inFile.is_open())
	{
		cerr << "Arquivo " << argv[1] << " nao existe!" << endl;
		return 1;
	}
	cout << "---------------------------------------------------" << endl;
	cout << "\t\tFILA DE INSTRUCOES" << endl;
	while(getline(inFile,line))
	{
		cout << line << endl;
		instruction_queue.push_back(line);
	}
	cout << "---------------------------------------------------" << endl << endl;
	inFile.close();
	inFile.open(argv[2]);
	if(!inFile.is_open())
	{
		cerr << "Arquivo " << argv[2] << " nao existe!" << endl;
		return 1;
	}
	cout << "---------------------------------------------------" << endl;
	cout << "VALOR INICIAL DOS REGISTRADORES" << endl;
	while(inFile >> value)
	{
		cout << 'R' << i << " = " << value << endl;
		status.push_back(value);
		i++;
	}
	cout << "---------------------------------------------------" << endl << endl;
	inFile.close();
	inFile.open(argv[3]);
	if(!inFile.is_open())
	{
		cerr << "Arquivo " << argv[3] << " nao existe!" << endl;
		return 1;
	}
	cout << "---------------------------------------------------" << endl;
	cout << "ESTADO INICIAL DA MEMORIA" << endl;
	i = 0;
	while(inFile >> value)
	{
		if(i%10 == 0)
			cout << endl << i << "\t||\t";
		cout << value << '\t';
		memoria.push_back(value);
		i++;
	}
	cout << endl << "---------------------------------------------------" << endl << endl;
	top top1("top",3,2,2,instruct_time,instruction_queue,status,&memoria);
	top1.clock(clock);
	sc_start(100,SC_NS);
	return 0;
}