NAME		= ircserv

CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -Weffc++ -Iincludes

SRCS		= src/main.cpp \
			  src/Server.cpp \
			  src/ServerException.cpp \
			  src/Client.cpp

OBJS		= $(SRCS:.cpp=.o)

RM			= rm -f

# =============================================================================
# RULES
# =============================================================================

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re