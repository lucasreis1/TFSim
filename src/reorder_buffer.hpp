#include "general.hpp"
#include "interfaces.hpp"
#include "branch_predictor.hpp"
#include <nana/gui/widgets/listbox.hpp>
#include<vector>
#include<deque>
#include<map>

using std::vector;
using std::deque;
using std::map;

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
    sc_port<write_if_f> out_adu;
    sc_port<read_if_f> in_slb;
    sc_port<write_if_f> out_slb;
    sc_port<write_if> out_iq;
    sc_port<write_if_f> out_resv_adu;
    sc_port<read_if_f> in_resv_adu;
    SC_HAS_PROCESS(reorder_buffer);
    reorder_buffer(sc_module_name name,unsigned int sz,unsigned int pred_size, nana::listbox &gui, nana::listbox::cat_proxy instr_gui);
    ~reorder_buffer();
    void leitura_issue();
    void new_rob_head();
    void leitura_cdb();
    void leitura_adu();
    void value_check();
    void check_conflict();

private:
    struct rob_slot{
        unsigned int entry;
        bool busy;
        string instruction;
        unsigned int state;
        string destination;
        float value;
        bool ready;
        bool prediction;
        int vj,vk;
        unsigned int qj,qk;
        unsigned int instr_pos;
        rob_slot(unsigned int id)
        {
            busy = ready = false;
            entry = id;
            qj = qk = 0;
        }
    };
    enum
    {
        ISSUE = 1,
        EXECUTE = 2,
        WRITE = 3,
        COMMIT = 4
    };
    unsigned int tam;
    unsigned int last_rob;
    rob_slot **ptrs;
    deque<rob_slot *> rob_buff;
    sc_event free_rob_event,new_rob_head_event,rob_head_value_event,resv_read_oper_event;
    branch_predictor preditor;
    map<string,unsigned int> branch_instr;
    nana::listbox &gui_table;
    nana::listbox::cat_proxy instr_queue_gui;

    int busy_check();
    unsigned int ask_status(bool read,string reg,unsigned int pos = 0);
    float ask_value(bool read,string reg,float value = 0);
    void mem_write(unsigned int addr,float value,unsigned int rob_pos);
    void check_dependencies(unsigned int index, float value);
    void _flush();
    bool branch(int optype,int rs = 0,int rt = 0);
    bool branch(int optype,float value);
    int instruction_pos_finder(string p);
};
