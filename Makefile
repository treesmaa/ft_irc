NAME := ft_irc

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -MMD -MP

INCLUDES := -Iincludes

SRC_MAIN 	:= src/main.cpp

SRC_SERVER	:= \
	src/server/Server.cpp

SRC_CLIENT	:= \
	src/client/Client.cpp

SRC_CHANNEL := \
	src/channel/Channel.cpp

SRC_COMMAND := \
	src/command/CommandParser.cpp \
	src/command/CommandHandler.cpp

SRC := \
	$(SRC_MAIN) \
	$(SRC_SERVER) \
	$(SRC_CLIENT) \
	$(SRC_CHANNEL) \
	$(SRC_COMMAND)

OBJS_DIR := obj/

OBJS := $(patsubst src/%.cpp,$(OBJS_DIR)%.o,$(SRC))
DEP := $(OBJS:.o=.d)

# --------------------------------------------------

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

$(OBJS_DIR)%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEP)

.PHONY: all clean fclean re