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
SERVICE := myserver
SUPERVISOR_CONF := /etc/supervisor/conf.d/$(SERVICE).conf
LOG_DIR := /var/log/$(SERVICE)

# Target to create the Supervisor configuration file
config: create-logs-dir
	@echo "Generating Supervisor configuration file $(SERVICE) at $(SUPERVISOR_CONF)..."
	@echo "[program:$(SERVICE)]" > $(SERVICE).conf.tmp
	@echo "command=$(shell pwd)/$(SERVER)" >> $(SERVICE).conf.tmp
	@echo "directory=$(shell pwd)" >> $(SERVICE).conf.tmp

	@echo "autostart=true" >> $(SERVICE).conf.tmp
	@echo "autorestart=true" >> $(SERVICE).conf.tmp
	@echo "startsecs=2" >> $(SERVICE).conf.tmp
	@echo "startretries=3" >> $(SERVICE).conf.tmp
	@echo "stopsignal=QUIT" >> $(SERVICE).conf.tmp
	@echo "stderr_logfile=/var/log/$(SERVICE)/$(SERVICE).err.log" >> $(SERVICE).conf.tmp
	@echo "stdout_logfile=/var/log/$(SERVICE)/$(SERVICE).out.log" >> $(SERVICE).conf.tmp
	@echo "user=$(USER)" >> $(SERVICE).conf.tmp
	@sudo mv $(SERVICE).conf.tmp $(SUPERVISOR_CONF)
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
	rm -f $(OBJS) $(SERVICE).conf.tmp spv.config

.PHONY: all create-logs-dir config start-server clean 