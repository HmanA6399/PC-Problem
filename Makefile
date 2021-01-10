producer:
	gcc -o producer producer.c

consumer:
	gcc -o consumer consumer.c

clean:
	rm -f producer consumer

all: clean producer consumer