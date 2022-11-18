EXEC_NAME = tfsim

ROOT=$(shell pwd)
# edit here to your systemC installation folder
SYSTEMC_LIB=$(ROOT)/systemc-2.3.3/build/src
SYSTEMC_INCLUDE=$(ROOT)/systemc-2.3.3/src
# edit here to your nana installation folder
NANA_INCLUDE=$(ROOT)/nana/include
NANA_LIB=$(ROOT)/nana/built

CXX = g++
INCLUDEDIR = include/
SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CPPFLAGS += -I $(SYSTEMC_INCLUDE) -I $(NANA_INCLUDE)
LDLFLAGS += -L $(NANA_LIB) -L $(SYSTEMC_LIB) -Wl,-rpath=$(SYSTEMC_LIB)

CFLAGS += -std=c++17 -Wall 
LIBS += -lnana -lX11 -lpthread -lrt -lXft -lpng -lasound -lfontconfig -lm -lsystemc

.PHONY: all nofs clean

all: EXE
nofs: EXE_NOFS

EXE: $(OBJ)
	$(CXX) $(LDLFLAGS) $^ $(LIBS) -lstdc++fs -o $(EXEC_NAME)

EXE_NOFS: $(OBJ)
	$(CXX) $(LDLFLAGS) $^ $(LIBS) -o $(EXEC_NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJ)
	rm $(EXEC_NAME)
