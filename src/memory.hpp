#include "interfaces.hpp"
#include "grid.hpp"

class memory: public sc_module
{
public:
    sc_port<read_if> in;
    sc_port<write_if> out;
    SC_HAS_PROCESS(memory);
    
    memory(sc_module_name name, nana::grid &m);
    void leitura_bus();
    
private:
    string p;
    nana::grid &mem;
};
