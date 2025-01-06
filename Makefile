ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME = libft_malloc.so

HOSTLIB = libft_malloc_$(HOSTTYPE).so

CC=gcc
FLAGS        = # -std=gnu99 -Iinclude
CFLAGS       = -fPIC -g -Wall -Wextra -g -Werror
LDFLAGS      = -shared
SRC=malloc.c
INCLUDES=malloc.h
OBJ=$(SRC:%.c=%.o)

all: $(NAME)

$(NAME) : $(HOSTLIB)
	@echo creating symbolic link
	ln -fs ${HOSTLIB} ${NAME}

${HOSTLIB}: ${OBJ}
	@echo compiling libft_malloc.so
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJ)

clean:
	rm -fr $(OBJ)

fclean: clean
	rm -rf $(NAME) $(HOSTLIB)

re: fclean all