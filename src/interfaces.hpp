#pragma once
#include<string>
#include<systemc.h>

using std::string;

class write_if_f: virtual public sc_interface
{
    public:
        virtual void write(string) = 0;
        virtual void nb_write(string) = 0;
};

class read_if_f: virtual public sc_interface
{
    public:
        virtual void read(string &) = 0;
        virtual void nb_read(string &) = 0;
        virtual void notify() = 0;
};

class write_if: virtual public sc_interface
{
    public:
        virtual void write(string) = 0;
};

class read_if: virtual public sc_interface
{
    public:
        virtual void read(string &) = 0;
};
