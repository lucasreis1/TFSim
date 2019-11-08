#include "interfaces.hpp"
#include<nana/gui/widgets/listbox.hpp>

using std::string;

class register_bank: public sc_module
{
public:
    sc_port<read_if_f> in;
    sc_port<write_if_f> out;
    sc_port<read_if> in_cdb;
    SC_HAS_PROCESS(register_bank);

    register_bank(sc_module_name name,nana::listbox &regs);
    void le_bus();
    void le_cdb();

private:
    nana::listbox &registers;
    enum
    {
        IVALUE = 1,
        IQ = 2,
        FVALUE = 4,
        FQ = 5
    };
};
