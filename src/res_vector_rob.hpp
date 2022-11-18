#include "interfaces.hpp"
#include "res_station_rob.hpp"
#include<map>
#include<vector>
#include<nana/gui/widgets/listbox.hpp>

using std::map;
using std::vector;

class res_vector_rob: public sc_module
{
public:
    vector<res_station_rob *> rs;
    sc_port<read_if_f> in_issue;
    sc_port<read_if> in_cdb;
    sc_port<write_if> out_cdb;
    sc_port<read_if_f> in_rb;
    sc_port<write_if_f> out_rb;
    sc_port<write_if> out_mem;
    sc_port<read_if_f> in_rob;
    sc_port<write_if_f> out_rob;
    SC_HAS_PROCESS(res_vector_rob);
    res_vector_rob(sc_module_name name,unsigned int t1, unsigned int t2,map<string,int> instruct_time, nana::listbox &lsbox, nana::listbox::cat_proxy ct, nana::listbox::cat_proxy r_ct);
    ~res_vector_rob();
    void leitura_issue();
    void leitura_rob();
private:
    map<string,int> res_type;
    unsigned int tam_pos[3];
    nana::listbox &table;
    //sc_event robFlushed;

    int busy_check(string inst);
    float ask_value(string reg);
    string ask_rob_value(string rob_pos);
    unsigned int ask_status(string reg);
};
