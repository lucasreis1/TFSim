#include<memory>
#include "bus.hpp"
#include "issue_control.hpp"
#include "clock_.hpp"
#include "res_vector.hpp"
#include "sl_buffer.hpp"
#include "register_bank.hpp"
#include "memory.hpp"
#include "instruction_queue.hpp"
//Bibliotecas para especulaçao
#include "issue_control_rob.hpp"
#include "res_vector_rob.hpp"
#include "sl_buffer_rob.hpp"
#include "reorder_buffer.hpp"
#include "register_bank_rob.hpp"
#include "memory_rob.hpp"
#include "instruction_queue_rob.hpp"
#include "address_unit.hpp"


using std::unique_ptr;

class top: public sc_module
{
public:
    top(sc_module_name name);
    void simple_mode(unsigned int nadd, unsigned int nmul,unsigned int nload,map<string,int> instruct_time,vector<string> instruct_queue, nana::listbox &table, nana::grid &mem_gui, nana::listbox &regs, nana::listbox &instr, nana::label &ccount);
    void rob_mode(unsigned int nadd, unsigned int nmul,unsigned int nload,map<string,int> instruct_time, vector<string> instruct_queue, nana::listbox &table, nana::grid &mem_gui, nana::listbox &regs, nana::listbox &instr, nana::label &count, nana::listbox &rob_gui);
private:
    //Para simple(sem especulacao)
    unique_ptr<bus> CDB,mem_bus,clock_bus;
    unique_ptr<cons_bus> inst_bus,rst_bus,sl_bus;
    unique_ptr<cons_bus_fast> rb_bus;
    unique_ptr<issue_control> iss_ctrl;
    unique_ptr<clock_> clk;
    unique_ptr<res_vector> rs_ctrl;
    unique_ptr<sl_buffer> slb;
    unique_ptr<register_bank> rb;
    unique_ptr<memory> mem;
    unique_ptr<instruction_queue> fila;
    //Para especulacao
    unique_ptr<bus> resv_rob_bus; //sincronizacao de leitura de operandos para evitar conflitos no reg_bank
    unique_ptr<bus> adu_bus,adu_sl_bus,mem_slb_bus,iq_rob_bus,rob_rt_bus;
    unique_ptr<cons_bus> rob_bus,ad_bus,rob_adu_bus;
    unique_ptr<cons_bus_fast> rob_slb_bus,rob_statval_bus;
    unique_ptr<address_unit> adu;
    unique_ptr<issue_control_rob> iss_ctrl_r;
    unique_ptr<reorder_buffer> rob;
    unique_ptr<res_vector_rob> rs_ctrl_r;
    unique_ptr<sl_buffer_rob> slb_r;
    unique_ptr<register_bank_rob> rb_r;
    unique_ptr<memory_rob> mem_r;
    unique_ptr<instruction_queue_rob> fila_r;
};
