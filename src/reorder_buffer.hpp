#include "general.hpp"
#include<vector>
#include<deque>

using std::vector;
using std::deque;

class reorder_buffer: public sc_module
{
public:
	sc_port<read_if_f> in_issue;
	sc_port<write_if_f> out_issue;
	sc_port<read_if> in_cdb;
	sc_port<read_if_f> in_rb;
	sc_port<write_if_f> out_rb;
	sc_port<write_if> out_mem;
	sc_port<read_if> in_adu;
	sc_port<read_if_f> in_slb;
	sc_port<write_if_f> out_slb;
	sc_port<write_if> out_iq;
	sc_port<write_if> out_resv;

	SC_HAS_PROCESS(reorder_buffer);
	reorder_buffer(sc_module_name name,unsigned int t,unsigned int t2);
	~reorder_buffer();
	void leitura_issue();
	void new_rob_head();
	void leitura_cdb();
	void leitura_adu();
	void check_conflict();

private:
	unsigned int tam;
	rob_slot **ptrs;
	deque<rob_slot *> rob_buff;
	sc_event free_rob_event,new_rob_head_event,rob_head_value_event;
	branch_predictor preditor;
	map<string,unsigned int> branch_instr;
	struct rob_slot{
		unsigned int entry;
		bool busy;
		string instruction;
		unsigned int state;
		string destination;
		float value;
		bool ready;
		unsigned int vj,vk;
		unsigned int qj,qk;
		rob_slot(unsigned int id)
		{
			busy = ready = false;
			entry = id;
			qj = qk = 0;
		}
	};

	int busy_check();
	unsigned int ask_status(bool read,string reg,unsigned int pos = 0);
	float ask_value(bool read,string reg,float value = 0);
	void mem_write(unsigned int addr,float value,unsigned int rob_pos);
	void check_dependencies(unsigned int index, float value);
	void _flush();
	bool branch(int optype,unsigned int rs = 0,unsigned int rt = 0);
	bool branch(int optype,float value);
};