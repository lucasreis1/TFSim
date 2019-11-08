#include "interfaces.hpp"
#include<map>
#include<vector>

using std::map;
using std::vector;

class issue_control: public sc_module
{
public:
    sc_port<read_if_f> in;
    sc_port<write_if_f> out_rsv;
    sc_port<write_if_f> out_slbuff;
    SC_HAS_PROCESS(issue_control);
    
    issue_control(sc_module_name name);
    void issue_select();

private:
    string p;
    vector<string> ord;
    map<string,unsigned short int> res_type;
};
