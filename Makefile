all:
	gcc main.c -o main
sanitized:
	gcc main.c -o main -fsanitize=address
