#include "interfaces.hpp"
#include "grid.hpp"

class memory_rob: public sc_module
{
public:
    sc_port<read_if> in;
    sc_port<write_if> out;
    sc_port<write_if> out_slb;
    SC_HAS_PROCESS(memory_rob);
    
    memory_rob(sc_module_name name, nana::grid &m);
    void leitura_bus();
    
private:
    string p;
    nana::grid &mem;
};
