all:
	gcc -std=gnu99 wtmpfilter.c -o wtmpfilter

clean:
	rm wtmpfilter
