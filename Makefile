all:
	gcc -I ./  sender.c -o sender -lpthread
	gcc -I ./  receiver.c -o receiver -lpthread

sender:
	gcc -I ./  sender.c -o sender -lpthread

receiver:
	gcc -I ./  receiver.c -o receiver -lpthread

receiver_clean:
	rm -f receiver

sender_clean:
	rm -f sender

clean:
	rm -f receiver
	rm -f sender

