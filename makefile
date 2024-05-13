make: main.c
	clang main.c -Og -ggdb3 -fsanitize=address
