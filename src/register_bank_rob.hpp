#include "interfaces.hpp"
#include<nana/gui/widgets/listbox.hpp>

using std::string;

class register_bank_rob: public sc_module
{
public:
    sc_port<read_if_f> in;
    sc_port<write_if_f> out;
    SC_HAS_PROCESS(register_bank_rob);

    register_bank_rob(sc_module_name name,nana::listbox &regs);
    void le_bus();

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
