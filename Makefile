main.exe: main.o bucket.o
	gcc main.o bucket.o -o main.exe -lsqlite3

main.o: src/main.c src/bucket.h
	gcc -c src/main.c

bucket.o: src/bucket.c src/bucket.h
	gcc -c src/bucket.c

.PHONY: clean
clean:
	rm *.o main.exe
	rm -rf build/
