CXX = g++
CXXFLAGS = -pthread -Wall
LDFLAGS = -lyaml-cpp -lboost_filesystem -lboost_system
INCLUDES = -Icommon -L/usr/include
COMMON_SRC = common/rpc.cpp common/config_parser.cpp common/utils.cpp

CONTROLLER_SRC = controller/main.cpp $(COMMON_SRC) controller/image_service.cpp
COMPUTE_SRC = compute/main.cpp $(COMMON_SRC)
CLIENT_SRC = client/main.cpp $(COMMON_SRC)

BIN_DIR = build

CONTROLLER_BIN = $(BIN_DIR)/controller
COMPUTE_BIN = $(BIN_DIR)/compute
CLIENT_BIN = $(BIN_DIR)/client

all: $(CONTROLLER_BIN) $(COMPUTE_BIN) $(CLIENT_BIN)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(CONTROLLER_BIN): $(CONTROLLER_SRC) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(CONTROLLER_SRC) $(LDFLAGS)

$(COMPUTE_BIN): $(COMPUTE_SRC) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(COMPUTE_SRC) $(LDFLAGS)

$(CLIENT_BIN): $(CLIENT_SRC) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(CLIENT_SRC) $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR)
