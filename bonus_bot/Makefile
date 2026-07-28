NAME := lilbot

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -MMD -MP

INCLUDES := -Iincludes

# ---------------- SOURCES ----------------

SRC := src/main.cpp src/Bot.cpp

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
