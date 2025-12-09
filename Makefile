CXX				= c++
CXXFLAGS		= -Wall -Wextra -Werror -std=c++98
NAME			= webserv

INCLUDES_DIR	= includes
SOURCES_DIR		= srcs
OBJECTS_DIR		= objs

SOURCES 		= $(SOURCES_DIR)/main.cpp \
				$(SOURCES_DIR)/Server.cpp \
				$(SOURCES_DIR)/Config.cpp \
				$(SOURCES_DIR)/Request.cpp \
				$(SOURCES_DIR)/Response.cpp \
				$(SOURCES_DIR)/Client.cpp \
				$(SOURCES_DIR)/CGI.cpp \
				$(SOURCES_DIR)/Utils.cpp

OBJECTS 		= $(SOURCES:$(SOURCES_DIR)/%.cpp=$(OBJECTS_DIR)/%.o)

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -I$(INCLUDES_DIR) -o $@ $^
	@echo "✓ Compilation successful!"

$(OBJECTS_DIR)/%.o: $(SOURCES_DIR)/%.cpp
	@mkdir -p $(OBJECTS_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDES_DIR) -c $< -o $@

clean:
	rm -rf $(OBJECTS_DIR)
	@echo "✓ Object files removed"

fclean: clean
	rm -f $(NAME)
	@echo "✓ All binaries removed"

re: fclean all
	@echo "✓ Rebuild successful!"

run: $(NAME)
	./$(NAME) config/default.conf

test: $(NAME)
	./$(NAME) config/test.conf
