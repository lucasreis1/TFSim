#include "top.hpp"

top::top(sc_module_name name): sc_module(name){}

void top::simple(unsigned int t1, unsigned int t2,unsigned int t3,map<string,int> instruct_time,vector<string> instruct_queue, nana::listbox &table, nana::grid &mem_gui, nana::listbox &regs, nana::listbox &instr, nana::label &ccount)
{
	CDB = unique_ptr<bus>(new bus("CDB"));
	mem_bus = unique_ptr<bus>(new bus("mem_bus"));
	clock_bus = unique_ptr<bus>(new bus("clock_bus"));
	inst_bus = unique_ptr<cons_bus>(new cons_bus("inst_bus"));
	rst_bus = unique_ptr<cons_bus>(new cons_bus("rst_bus"));
	sl_bus = unique_ptr<cons_bus>(new cons_bus("sl_bus"));
	rb_bus = unique_ptr<cons_bus_fast>(new cons_bus_fast("rb_bus"));
	iss_ctrl = unique_ptr<issue_control>(new issue_control("issue_control"));
	clk = unique_ptr<clock_>(new clock_("clock",1,ccount));
	fila = unique_ptr<instruction_queue>(new instruction_queue("fila_inst",instruct_queue,instr));
	rst = unique_ptr<res_vector>(new res_vector("rs_vc",t1,t2,instruct_time,table,instr.at(0)));
	rb = unique_ptr<register_bank>(new register_bank("register_bank", regs));
	slb = unique_ptr<sl_buffer>(new sl_buffer("sl_buffer",t3,t1+t2,instruct_time,table,instr.at(0)));
	mem = unique_ptr<memory>(new memory("memoria", mem_gui));
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