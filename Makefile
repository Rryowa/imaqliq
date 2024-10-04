CXX = g++
CXXFLAGS = -Werror -Wall -Wextra -g
INCLUDES = -I./helper

# Source files
HELPER_SRCS = helper/file.cpp helper/receiver.cpp
SRC_SRCS = src/client.cpp src/server.cpp
SRCS = $(HELPER_SRCS) $(SRC_SRCS)

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executables
CLIENT = client
SERVER = server

all: $(CLIENT) $(SERVER) config start-server clean

$(CLIENT): src/client.o $(filter-out src/server.o, $(OBJS))
	$(CXX) $(CXXFLAGS) $^ -o $@

$(SERVER): src/server.o $(filter-out src/client.o, $(OBJS))
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

USER := $(shell whoami)
include .env

# Target to create the Supervisor configuration file
config: create-logs-dir
	@echo "Generating Supervisor configuration file $(SERVICE) at $(SUPERVISOR_CONF)..."
	@echo "[program:$(SERVICE)]" > $(SERVICE_CONF)
	@echo "command=$(shell pwd)/$(SERVER)" >> $(SERVICE_CONF)
	@echo "directory=$(shell pwd)" >> $(SERVICE_CONF)

	@echo "autostart=false" >> $(SERVICE_CONF)
	@echo "autorestart=false" >> $(SERVICE_CONF)
	@echo "startsecs=2" >> $(SERVICE_CONF)
	@echo "startretries=3" >> $(SERVICE_CONF)
	@echo "stopsignal=QUIT" >> $(SERVICE_CONF)
	@echo "stderr_logfile=$(ERR_LOG)" >> $(SERVICE_CONF)
	@echo "stdout_logfile=$(OUT_LOG)" >> $(SERVICE_CONF)
	@echo "user=$(USER)" >> $(SERVICE_CONF)
	@sudo mv $(SERVICE_CONF) $(SUPERVISOR_CONF)
	@sudo supervisorctl reread
	@sudo supervisorctl update
	@echo "Supervisor configuration for $(SERVICE) installed successfully at $(SUPERVISOR_CONF)."

start-server:
	@echo "Starting $(SERVICE) using supervisorctl"
	@sudo supervisorctl restart $(SERVICE)

# Target to create log directories with appropriate permissions
create-logs-dir:
	@echo "Creating log directory at $(LOG_DIR)"
	@sudo mkdir -p $(LOG_DIR)
	@sudo chown $(USER):$(USER) $(LOG_DIR)
	@sudo chmod 755 $(LOG_DIR)

clean:
	rm -f $(OBJS) $(SERVICE_CONF) spv.config

.PHONY: all create-logs-dir config start-server clean 