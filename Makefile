smtp:
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -oEmailSender.o EmailSender.cpp
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -osmtp.o smtp.cpp
	g++ -osmtp smtp.o EmailSender.o -lssl -lcrypto

clean:
	rm smtp *.o
