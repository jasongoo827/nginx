.DEFAULT_GOAL := all

NAME = webserv

CC = c++

CFLAGS = -Wall -Werror -Wextra -Iinclude -std=c++98 -fsanitize=address -g3

RM = rm -rf

SRCS = $(wildcard srcs/*.cpp)

OBJS = $(SRCS:%.cpp=%.o)

DEPS_DIR = dep
DEPS = $(SRCS:%.cpp=$(DEPS_DIR)/%.d)

$(shell mkdir -p $(DEPS_DIR)/srcs)

-include $(DEPS)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
	@mv -f $*.d $(DEPS_DIR)/$*.d

# $(DEPS_DIR)/%.d: %.cpp
# 	$(CC) $(CFLAGS) -MMD -MP -c $< -o $(<:%.cpp=%.o)
# 	@mv -f $(<:%.cpp=%.d) $(DEPS_DIR)/$*.d

clean:
	$(RM) $(OBJS)
	$(RM) $(wildcard $(DEPS_DIR)/srcs/*.d)
	$(RM) $(DEPS_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
