NAME=malloc
CC=gcc
CFLAG=-W -W -W
SRC=malloc.c main.c
OBJ=$(SRC:%.c=%.o)

all: $(NAME)

$(NAME) : $(OBJ)
	$(CC) $(CFLAG) -o $(NAME) $(OBJ)

clean:
	rm -fr $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all