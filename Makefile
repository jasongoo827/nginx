.DEFAULT_GOAL := all

NAME = webserv

CC = c++

CFLAGS = -Wall -Werror -Wextra -Iinclude -std=c++98 #-fsanitize=address -g3

RM = rm -rf

SRCS = $(wildcard srcs/Config/*.cpp) \
		$(wildcard srcs/*.cpp) \
		$(wildcard srcs/Request/*.cpp) \
		$(wildcard srcs/yakim/*.cpp)

OBJS = $(SRCS:%.cpp=%.o)

DEPS_DIR = srcs/deps
DEPS = $(SRCS:%.cpp=$(DEPS_DIR)/%.d)

$(shell mkdir -p $(DEPS_DIR))

-include $(DEPS)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ 

$(DEPS_DIR)/%.d: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $(<:%.cpp=%.o)
	@mv -f $(<:%.cpp=%.d) $(DEPS_DIR)/$*.d

clean:
	$(RM) $(OBJS)
	$(RM) $(DEPS_DIR)/%.d

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
