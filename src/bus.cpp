#include "bus.hpp"
        
bus::bus(sc_module_name name): sc_channel(name)
{
    stamp = sc_time(-1,SC_NS);
}
void bus::write(string p)
{
    string p_back = p;
    while(sc_time_stamp() == stamp)
        wait(sc_time(1,SC_NS));
    stamp = sc_time_stamp();
    wait(SC_ZERO_TIME);
    palavra = p_back;
    write_event.notify();
}
const sc_event& bus::default_event() const
{
    return write_event;
}
void bus::read(string &p)
{
    p = palavra;
}

cons_bus::cons_bus(sc_module_name name): sc_channel(name)
{
    empty = true;
}       
void cons_bus::write(string p)
{
    if(!empty)
        wait(read_event);
    empty = false;
    palavra = p;
    write_event.notify(SC_ZERO_TIME);
    wait(read_event);
}
void cons_bus::nb_write(string p)
{
    if(empty)
    {
        empty = false;
        palavra = p;
    }
}
void cons_bus::read(string &p)
{
    if(empty)
        wait(write_event);
    empty = true;
    p = palavra;
    read_event.notify(SC_ZERO_TIME);
    palavra = " ";
}
void cons_bus::nb_read(string &p)
{
    if(!empty)
        p  = palavra;
}
void cons_bus::notify()
{
    empty = true;
    read_event.notify(SC_ZERO_TIME);
}
const sc_event& cons_bus::default_event() const
{
    return write_event;
}

cons_bus_fast::cons_bus_fast(sc_module_name name): sc_channel(name)
{
    empty = true;
}
void cons_bus_fast::write(string p)
{
    if(!empty)
        wait(read_event);
    empty = false;
    palavra = p;
    write_event.notify();
    wait(read_event);
}
void cons_bus_fast::nb_write(string p)
{
    if(empty)
    {
        empty = false;
        palavra = p;
    }
}
void cons_bus_fast::read(string &p)
{
    if(empty)
        wait(write_event);
    empty = true;
    p = palavra;
    palavra = " ";
    read_event.notify();
}
void cons_bus_fast::nb_read(string &p)
{
    if(!empty)
        p  = palavra;
}
void cons_bus_fast::notify()
{
    empty = true;
    palavra = " ";
    read_event.notify();
}
const sc_event& cons_bus_fast::default_event() const
{
    return write_event;
}
