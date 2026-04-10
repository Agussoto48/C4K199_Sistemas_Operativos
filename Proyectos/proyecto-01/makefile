CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude/

SRC = src/*.cpp	
EXE = bin/rotonda


default: build

BIN_DIR = bin
# Crear bin si no existe
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)


build: | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXE)

run:
	@$(EXE)

clean:
	@rm -rf $(BIN_DIR)