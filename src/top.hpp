#include<memory>
#include "bus.hpp"
#include "issue_control.hpp"
#include "clock_.hpp"
#include "res_vector.hpp"
#include "sl_buffer.hpp"
#include "register_bank.hpp"
#include "memory.hpp"
#include "instruction_queue.hpp"

using std::unique_ptr;

class top: public sc_module
{
public:
	top(sc_module_name name);
	void simple(unsigned int t1, unsigned int t2,unsigned int t3,map<string,int> instruct_time,vector<string> instruct_queue, nana::listbox &table, nana::grid &mem_gui, nana::listbox &regs, nana::listbox &instr, nana::label &ccount);
private:
	unique_ptr<bus> CDB,mem_bus,clock_bus;
	unique_ptr<cons_bus> inst_bus,rst_bus,sl_bus;
	unique_ptr<cons_bus_fast> rb_bus;
	unique_ptr<issue_control> iss_ctrl;
	unique_ptr<clock_> clk;
	unique_ptr<res_vector> rst;
	unique_ptr<sl_buffer> slb;
	unique_ptr<register_bank> rb;
	unique_ptr<memory> mem;
	unique_ptr<instruction_queue> fila;
};