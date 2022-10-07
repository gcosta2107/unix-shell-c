CC = gcc

objects = shell.o functions.o

output: $(objects)
	$(CC) -o shell shell.o functions.o

shell.o: functions.h

utils.o: functions.h

.PHONY: clean
clean:
	rm $(objects) shell
	