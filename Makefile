
CFLAGS =-std=c++14 -lnana -lX11 -lpthread -lrt -lXft -lpng -lasound -lfontconfig -lstdc++fs
SYSTEMCFLAGS = -g -ggdb -Wall -I /opt/systemc/include -L /opt/systemc/lib -Wl,-rpath=/opt/systemc/lib -lm -lsystemc

default:
	g++-6 $(wildcard *.cpp) -o t $(CFLAGS) $(SYSTEMCFLAGS)
