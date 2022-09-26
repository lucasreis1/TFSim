#include "clock_.hpp"

clock_::clock_(sc_module_name name, int dl, nana::label &clk):  sc_module(name), delay(dl), clock_count(clk)
{
    SC_THREAD(main);
}

void clock_::main()
{
    while(true)
    {
        sc_pause();
        wait(SC_ZERO_TIME);
        out->write("");
        clock_count.caption(std::to_string(sc_time_stamp().value() / 1000));
        wait(delay,SC_NS);
    }
}
