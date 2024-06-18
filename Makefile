.DEFAULT_GOAL := all

NAME = webserv

CC = c++

CFLAGS = -Wall -Werror -Wextra -g -MMD -MP -std=c++98

RM = rm -f

SRCS = srcs/Config/Config.cpp srcs/Config/Locate.cpp srcs/Config/Server.cpp srcs/main.cpp srcs/Status.cpp srcs/Utils.cpp


OBJS = $(SRCS:%.cpp=%.o)

DEPS = $(SRCS:%.cpp=%.d)

-include $(DEPS)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o	: %.cpp
	$(CC) $(CFLAGS)  -c -o $@ $<

clean:
	$(RM) $(OBJS) $(DEPS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
