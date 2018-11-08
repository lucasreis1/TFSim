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
	instruction_queue_rob(sc_module_name name, vector<string> inst_q, nana::listbox &instr);
	void main();
	void leitura_rob();

private:
	unsigned int pc,last_pc,next_pc;
	vector<string> instruct_queue;
	nana::listbox &instructions;
};