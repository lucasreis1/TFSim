EXEC_NAME = tfsim

C++ = g++
INCLUDEDIR = include/
SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CPPFLAGS += -I /opt/systemc/include 
LDLFLAGS += -L /opt/systemc/lib -Wl,-rpath=/opt/systemc/lib 
CFLAGS += -std=c++17 -Wall 
LIBS += -lnana -lX11 -lpthread -lrt -lXft -lpng -lasound -lfontconfig -lm -lsystemc

.PHONY: all nofs clean

all: EXE
nofs: EXE_NOFS

EXE: $(OBJ)
	$(C++) $(LDLFLAGS) $^ $(LIBS) -lstdc++fs -o $(EXEC_NAME)

EXE_NOFS: $(OBJ)
	$(C++) $(LDLFLAGS) $^ $(LIBS) -o $(EXEC_NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(C++) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJ)
	rm $(EXEC_NAME)
