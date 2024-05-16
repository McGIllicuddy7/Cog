make: main.c
	clang main.c  -ggdb3 -fsanitize=address
