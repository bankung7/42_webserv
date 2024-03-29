NAME = webserv

CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98

RM = rm -rf

INCS = -Iincs

SRCS_DIR = srcs/
SRCS = main.cpp Webserv.cpp Epoll.cpp \
	Server.cpp HttpHandler.cpp CGI.cpp \
	StrUtils.cpp Conf.cpp

OBJS_DIR = objs/
OBJS = $(addprefix $(OBJS_DIR), $(SRCS:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJS_DIR)%.o: $(SRCS_DIR)%.cpp
	@mkdir -p $(OBJS_DIR)
	$(CC) $(CFLAGS) -c $^ -o $@ $(INCS)

clean:
	$(RM) $(OBJS_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

r: re
	./$(NAME)

.PHONY: all clean fclean re r