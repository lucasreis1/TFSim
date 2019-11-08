#pragma once
#include "interfaces.hpp"

class bus: public sc_channel, public write_if, public read_if
{
public:
    bus(sc_module_name name);
    void write(string p);
    const sc_event& default_event() const;
    void read(string &p);

private:
    string palavra;
    sc_event write_event;
    sc_time stamp;
};

class cons_bus: public sc_channel, public write_if_f, public read_if_f
{
public:
    cons_bus(sc_module_name name);      
    void write(string p);
    void nb_write(string p);
    void read(string &p);
    void nb_read(string &p);
    void notify();
    const sc_event& default_event() const;

private:
    bool empty;
    string palavra;
    sc_event write_event;
    sc_event read_event;
};

class cons_bus_fast: public sc_channel, public write_if_f, public read_if_f
{
public:
    cons_bus_fast(sc_module_name name);
    void write(string p);
    void nb_write(string p);
    void read(string &p);
    void nb_read(string &p);
    void notify();
    const sc_event& default_event() const;

private:
    bool empty;
    string palavra;
    sc_event write_event;
    sc_event read_event;
};
