#include<systemc.h>
#include<vector>
#include "interfaces.hpp"
#include<nana/gui/widgets/listbox.hpp>

using std::vector;

class instruction_queue_rob: public sc_module
{
public:
	sc_port<read_if> in;
	sc_port<write_if_f> out;
	sc_port<read_if> in_rob;

	SC_HAS_PROCESS(instruction_queue_rob);
	instruction_queue_rob(sc_module_name name, vector<string> inst_q,int rb_sz, nana::listbox &instr);
	void main();
	void leitura_rob();
	void clear_gui(unsigned int pos);
	void replace_instructions(unsigned int pos,unsigned int index);
	void add_instructions(unsigned int pos, vector<string> instructions);


private:
	unsigned int pc;
	vector<string> instruct_queue;
	vector<string> original_instruct;
	vector<vector<string>> last_instr;
	vector<unsigned int> last_pc;
	nana::listbox &instructions;
};
