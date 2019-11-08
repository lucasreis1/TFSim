#include "general.hpp"
#include "res_station.hpp"
#include<vector>
#include<map>

using std::vector;
using std::map;

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
    
    res_vector(sc_module_name name,unsigned int t1, unsigned int t2,map<string,int> instruct_time, nana::listbox &lsbox, nana::listbox::cat_proxy ct);
    ~res_vector();
    void leitura_issue();

private:
    map<string,int> res_type;
    unsigned int tam_pos[3];
    nana::listbox &table;

    int busy_check(string inst);
    float ask_value(string reg);
    unsigned int ask_status(bool read,string reg,unsigned int pos = 0);
};
