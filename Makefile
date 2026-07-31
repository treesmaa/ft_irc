NAME := ircserv

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -MMD -MP

INCLUDES := -Iincludes

# ---------------- SOURCES ----------------

SRC_MAIN := src/main.cpp

SRC_SERVER := src/server/Server.cpp

SRC_CLIENT := src/client/Client.cpp

SRC_CHANNEL := src/channel/Channel.cpp

SRC_COMMAND := \
	src/command/CommandHandler.cpp \
	src/command/CommandParser.cpp

SRC_SIGNAL := src/signal/Signal.cpp

SRC_DEBUG := src/debug/Debug.cpp

SRC := \
	$(SRC_MAIN) \
	$(SRC_SERVER) \
	$(SRC_CLIENT) \
	$(SRC_CHANNEL) \
	$(SRC_COMMAND) \
	$(SRC_SIGNAL) \
	$(SRC_DEBUG)

# ---------------- OBJECTS ----------------

OBJS_DIR := obj
OBJS := $(patsubst src/%.cpp,$(OBJS_DIR)/%.o,$(SRC))
DEPS := $(OBJS:.o=.d)

# ---------------- RULES ----------------

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJS_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# ---------------- DEBUG BUILD ----------------

debug: CXXFLAGS += -g -DDEBUG
debug: fclean $(NAME)

# ---------------- CLEAN ----------------

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

# ---------------- DEPENDENCIES ----------------

-include $(DEPS)

.PHONY: all debug clean fclean re