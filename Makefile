main.exe: main.o
	gcc main.o -o main.exe -lsqlite3

main.o: src/main.c
	gcc -c src/main.c

.PHONY: clean
clean:
	rm *.o main.exe
	rm -rf build/
