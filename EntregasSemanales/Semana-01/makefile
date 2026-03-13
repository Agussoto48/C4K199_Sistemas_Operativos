CXX = g++
CXXFLAGS = -Wall -std=c++17

BIN_DIR = bin

SRC_BUZON = shared/Buzon.cpp

#Archivos fuentes de enviarConclases/recibir
SRC_SND_CC = conClases/enviarConClases.cpp $(SRC_BUZON)
SRC_RCV_CC = conClases/recibirConClases.cpp $(SRC_BUZON)
#Archivos Fuentes de enviar/recibir sin clases
SRC_SND_SC = sinClases/enviarSinClases.cpp 
SRC_RCV_SC = sinClases/recibirSinClases.cpp



# Ejecutables
EXE_SND_CC = $(BIN_DIR)/enviarClase
EXE_RCV_CC = $(BIN_DIR)/recibirClase

EXE_SND_SC = $(BIN_DIR)/enviar
EXE_RCV_SC = $(BIN_DIR)/recibir

# Regla por defecto
default: buildClase buildSinClase

# Crear bin si no existe
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

buildClase: | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRC_SND_CC) -o $(EXE_SND_CC)
	$(CXX) $(CXXFLAGS) $(SRC_RCV_CC) -o $(EXE_RCV_CC)

buildSinClase: | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRC_SND_SC) -o $(EXE_SND_SC)
	$(CXX) $(CXXFLAGS) $(SRC_RCV_SC) -o $(EXE_RCV_SC)

runEC:
	@./$(EXE_SND_CC)

runRC:
	@./$(EXE_RCV_CC)

runE:
	@./$(EXE_SND_SC)

runR:
	@./$(EXE_RCV_SC)

clean:
	@rm -rf $(BIN_DIR)
