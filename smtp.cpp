#include <winsock2.h>
#include <string>
#include <iostream>

#include <openssl/ssl.h>
#include <openssl/err.h>

using namespace std;

#include "EmailSender.h"

int main(int argc, char *argv[])
{
	EmailSender es;
	if (es.parseInputFile())
		es.sendEmail();
	else
		cout << "File to provide input params not found!\n" << endl;
	
	return 0;
}