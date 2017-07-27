all:
	gcc -I ./  sender.c -o sender -lpthread
	gcc -I ./  receiver.c -o receiver -lpthread

sender:
	gcc -I ./  sender.c -o sender -lpthread

receiver:
	gcc -I ./  receiver.c -o receiver -lpthread

receiver_clean:
	rm receiver

sender_clean:
	rm sender

clean:
	rm receiver
	rm sender

