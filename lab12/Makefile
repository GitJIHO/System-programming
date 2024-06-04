CC = gcc

CFLAGS = -lpthread

TARGETS = incrprint twordcount2 twordcount3 twordcount4 twordcount2-rev twordcount3-rev

all: $(TARGETS)

incrprint: incrprint.c
	$(CC) $< -o $@ $(CFLAGS)

twordcount2: twordcount2.c
	$(CC) $< -o $@ $(CFLAGS)

twordcount3: twordcount3.c
	$(CC) $< -o $@ $(CFLAGS)

twordcount4: twordcount4.c
	$(CC) $< -o $@ $(CFLAGS)

twordcount2-rev: twordcount2-rev.c
	$(CC) $< -o $@ $(CFLAGS)

twordcount3-rev: twordcount3-rev.c
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm -f $(TARGETS)