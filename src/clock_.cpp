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
        clock_count.caption(sc_time_stamp().to_string());
        wait(delay,SC_NS);
    }
}
