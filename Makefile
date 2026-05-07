NAME := ft_irc

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -MMD -MP

INCLUDES := -Iincludes

SRC := \
	src/main.cpp \
	src/server/Server.cpp \
	src/client/Client.cpp \
	src/channel/Channel.cpp \
	src/command/CommandParser.cpp \
	src/command/CommandHandler.cpp

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