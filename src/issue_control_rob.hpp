#include "interfaces.hpp"
#include<map>
#include<vector>

using std::map;
using std::vector;

class issue_control_rob: public sc_module
{
public:
    sc_port<read_if_f> in;
    sc_port<write_if_f> out_rsv;
    sc_port<write_if_f> out_slbuff;
    sc_port<read_if_f> in_slbuff;
    sc_port<read_if_f> in_rob;
    sc_port<write_if_f> out_rob;
    sc_port<write_if_f> out_adu;
    SC_HAS_PROCESS(issue_control_rob);
    
    issue_control_rob(sc_module_name name);
    void issue_select();

private:
    string p,rob_pos;
    vector<string> ord;
    map<string,unsigned short int> res_type;
};
