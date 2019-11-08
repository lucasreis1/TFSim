#pragma once
#include "bus.hpp"
#include<nana/gui/widgets/label.hpp>

class clock_: public sc_module
{
public:
    sc_port<write_if> out;
    SC_HAS_PROCESS(clock_); 
    clock_(sc_module_name name, int dl, nana::label &clk);
    void main();
private:
    int delay;
    nana::label &clock_count;
};
