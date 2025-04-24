# =============================================================================
# CONFIGURATION
# =============================================================================

NAME        = ircserv

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98 -Iincludes -pedantic-errors -Wno-c++98-compat-pedantic -Wno-padded

SRCDIR      = src
OBJDIR      = build

SRCFILES    = main.cpp \
              Server.cpp \
              ServerException.cpp \
              Client.cpp \
              Channel.cpp \
              CommandHandler.cpp \
              utils.cpp \
              action_join.cpp \
              action_nick.cpp \
              action_pass.cpp \
              action_user.cpp \
			  action_privmsg.cpp \
              action_topic.cpp

SRCS        = $(addprefix $(SRCDIR)/, $(SRCFILES))
OBJS        = $(addprefix $(OBJDIR)/, $(SRCFILES:.cpp=.o))

RM          = rm -f
MKDIR_P     = mkdir -p

# =============================================================================
# RULES
# =============================================================================

all: $(NAME)

# Link final
$(NAME): $(OBJS)
	@echo "\033[0;32m[LD] $@\033[0m"
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# Compilation
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@$(MKDIR_P) $(dir $@)
	@echo "\033[0;32m[CXX] $<\033[0m"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Nettoyage
clean:
	$(RM) -r $(OBJDIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
