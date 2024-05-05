all: play_again1 play_again2

play_again1: play_again1.c
	gcc -o playagain1 $^

play_again2: play_again2.c
	gcc -o playagain2 $^

clean:
	rm playagain1 playagain2
