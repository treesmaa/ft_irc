NAME := ft_irc
CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98 -MMD -MP
INCLUDES = -Iincludes

RM = rm -rf

SRC_DIR = src
OBJ_DIR = obj

SRC = main.cpp \

SRCS = $(addprefix $(SRC_DIR)/, $(SRC))
OBJ  = $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))
DEP  = $(OBJ:.o=.d)

all: $(NAME)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

-include $(DEP)

.PHONY: all clean fclean re
