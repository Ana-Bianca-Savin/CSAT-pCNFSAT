build:
	gcc -std=c99 -Wall -o CNFSAT main.c functiiArbore.c
run:
	./CNFSAT $(INPUT) $(OUTPUT)
clean:
	rm CNFSAT