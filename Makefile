ifeq ($(HOSTTYPE),)
	HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME = libft_malloc.so

HOSTLIB = libft_malloc_$(HOSTTYPE).so

CC				= gcc
CFLAGS       	= -Wall -Wextra -g -Werror -I/
LDFLAGS      	= -shared
SRC				= malloc.c utility.c
INCLUDES		= malloc.h
MK_OBJ_FLAGS 	= -fPIC
OBJ=$(SRC:%.c=%.o)

all: $(NAME)

test:
	$(CC) $(CFLAGS) $(SRC) main.c -o malloc && ./malloc

$(NAME) : $(HOSTLIB)
	@echo creating symbolic link
	ln -fs ${HOSTLIB} ${NAME}

${HOSTLIB}: ${OBJ} ${INCLUDES}
	@echo compiling libft_malloc.so
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) -o $(HOSTLIB)

%.o: %.c
	$(CC) $(CFLAGS) $(MK_OBJ_FLAGS) -c $< -o $@

clean:
	rm -fr $(OBJ)

fclean: clean
	rm -rf $(NAME) $(HOSTLIB)

re: fclean all