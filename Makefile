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

all: $(CLIENT) $(SERVER) clean

$(CLIENT): src/client.o $(filter-out src/server.o, $(OBJS))
	$(CXX) $(CXXFLAGS) $^ -o $@

$(SERVER): src/server.o $(filter-out src/client.o, $(OBJS))
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

.PHONY: all clean