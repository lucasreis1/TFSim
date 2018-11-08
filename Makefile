EXE = t

C++ = g++
INCLUDEDIR = include/
SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CPPFLAGS += -I /opt/systemc/include 
LDLFLAGS += -L /opt/systemc/lib -Wl,-rpath=/opt/systemc/lib 
CFLAGS += -std=c++14 -g -ggdb -Wall 
LIBS += -lnana -lX11 -lpthread -lrt -lXft -lpng -lasound -lfontconfig -lstdc++fs -lm -lsystemc

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(C++) $(LDLFLAGS) $^ $(LIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(C++) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJ)