#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "EmailSender.h"

static const string base64_chars ="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

string EmailSender::base64Encode(char const* bytes_to_encode, unsigned int in_len)
{
	string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i <4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';
	}

	return ret;
}

EmailSender::EmailSender()
{

}

vector<string> EmailSender::tokenize(string str, string delimiters) {
	vector<string> v;
	int start = 0;
	int pos = str.find_first_of(delimiters, start);
	while (pos != string::npos) {
		if (pos != start) // ignore empty tokens
			v.push_back(str.substr(start, pos-start));
		start = pos + 1;
		pos = str.find_first_of(delimiters, start);
	}
	if (start < str.length()) // ignore trailing delimiter
		v.push_back(str.substr(start, str.length() - start)); // add what's left of the string
	return v;
}

bool EmailSender::parseInputFile()
{
	bool bRet = true;

	// Look for the file which contains all details...
	string line;
	ifstream pfile("params.txt");
	if (pfile.is_open())
	{
		while (getline(pfile, line))
		{
			vector<string> v = tokenize(line, "=");
			if (v[0] == "SMTPServer")
			{
				m_sSmtpServerName = v[1];
			}
			else if (v[0] == "FromAddress")
			{
				m_sFromAddr = v[1];
			}
			else if (v[0] == "ToAddress")
			{
				m_sToAddr = v[1];
			}
			else if (v[0] == "Subject")
			{
				m_sSubject = v[1];
			}
			else if (v[0] == "MainContentFile")
			{
				m_sMainContentFile = v[1];
			}
			else if (v[0] == "AttachmentFile")
			{
				m_sAttachmentFile = v[1];
			}
		}
		pfile.close();
		cout << m_sSmtpServerName << " " << m_sFromAddr << " " << m_sToAddr << " " << m_sSubject << endl;

		// Encode Username...
		m_sEncrUserName = base64Encode(m_sFromAddr.c_str(), m_sFromAddr.length());

		// Take Password and encode it...
		string sPwd;
		cout << "Enter Password : ";
		getline(cin, sPwd);
		m_sEncrPwd = base64Encode(sPwd.c_str(), sPwd.length());
	}
	else
	{
		bRet = false;
	}
	return bRet;
}

int EmailSender::connectToSMTPServer()
{

	struct hostent*   lpHostEntry;
	struct servent*   lpServEntry;
	struct sockaddr_in SockAddr;

	// Attempt to intialize WinSock (1.1 or later)
#ifdef _WIN32
	WSADATA     WSData;
	if (WSAStartup(MAKEWORD(2, 2), &WSData))
	{
		cout << "Cannot find Winsock v2" << endl;
		return -1;
	}
#endif

	cout << "calling gethostbyname for " << m_sSmtpServerName << endl;

	struct addrinfo hints, *servinfo;
	memset(&hints, 0, sizeof hints);
	int rv = 0;
	hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(m_sSmtpServerName.c_str(), "smtp", &hints, &servinfo)) != 0) {
	    cout << "getaddrinfo: " << gai_strerror(rv) << endl;
	    return -1;
	}

	cout << "After getaddrinfo" << endl;

	// Create a TCP/IP socket, no specific protocol
	m_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (m_socket == -1)
	{
		cout << "Cannot open mail server socket" << endl;
		return -1;
	}

	// Get the mail service port
	lpServEntry = getservbyname("mail", 0);

	// Use the SMTP default port if no other port is specified
	int iProtocolPort;
	if (!lpServEntry)
		iProtocolPort = htons(587);
	else
		iProtocolPort = lpServEntry->s_port;

	// Connect the Socket...
	if (connect(m_socket, servinfo->ai_addr, servinfo->ai_addrlen))
	{
		cout << "Error connecting to Server socket" << endl;
		return -1;
	}

	// 1 Sec Timeout
	timeval tv;
#ifdef _WIN32
	tv.tv_sec = 1000;
#else
	tv.tv_sec = 1;
#endif
	tv.tv_usec = 0;
	setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &tv, sizeof(tv));

	cout << "Connection established successfully!" << endl;
	return 1;
}

int EmailSender::receiveDataFromServer(bool bSecured)
{
	char szBuffer[8192];
	memset(szBuffer, 0, sizeof(szBuffer));
	int ret = 0;

	// recv response from the server...
	do
	{
		if (bSecured)
			ret = SSL_read(m_sslHandle, szBuffer, sizeof(szBuffer));
		else
			ret = recv(m_socket, szBuffer, sizeof(szBuffer), 0);
		if (ret > 0)
			cout << szBuffer;
	} while (ret > 0);

	cout << endl;
	return 1;
}

int EmailSender::exchangeMsgWithServer(string sMessage)
{
	cout << "Client Says : " << sMessage << endl;
	int ret = send(m_socket, sMessage.c_str(), sMessage.length(), 0);
	if (ret < 0)
		return -1;
	else
	{
		cout << "Server Says :" << endl;
		receiveDataFromServer(false);
	}

	return 1;
}

int EmailSender::establishSecureConnection()
{
	// ssl Init...
	SSL_CTX *sslContext;

	// Register the error strings for libcrypto & libssl
	SSL_load_error_strings();

	// Register the available ciphers and digests
	SSL_library_init();

	// New context saying we are a client, and using SSL 2 or 3
	sslContext = SSL_CTX_new(SSLv23_client_method());
	if (sslContext == NULL)
	{
		cout << "Error in setting up sslcontext" << endl;
		return -1;
	}

	// Create an SSL struct for the connection
	m_sslHandle = SSL_new(sslContext);
	if (m_sslHandle == NULL)
	{
		cout << "Error in creating SSL" << endl;
		return -1;
	}

	// Connect the SSL struct to our connection
	int res = SSL_set_fd(m_sslHandle, m_socket);
	if (!res)
	{
		cout << "Error in setting FD" << endl;
		return -1;
	}

	// Initiate SSL handshake
	res = SSL_connect(m_sslHandle);
	if (res != 1)
	{
		int ret = 0;
		int retVal = SSL_get_error(m_sslHandle, ret);
		if (retVal == SSL_ERROR_SSL)
		{
			char szDebug[256];
			ERR_error_string(retVal, szDebug);
			cout << "Could not do SSL Handshake.. Error = " << szDebug << endl;
		}
		return -1;
	}

	cout << "SSL handshake Successful" << endl;
	return 1;
}

int EmailSender::exchangeMsgSecurelyWithServer(string sMessage)
{
	cout << "Client Says : " << sMessage << endl;
	int ret = SSL_write(m_sslHandle, sMessage.c_str(), sMessage.length());
	if (ret < 0)
		return -1;
	else
	{
		cout << "Server Says :" << endl;
		receiveDataFromServer(true);
	}
	return 1;
}

int EmailSender::sendEmail()
{
	cout << "Connecting to the SMTP server..." << endl;
	// Create Socket and connect to the server...
	int ret = connectToSMTPServer();
	if (ret > 0)
	{
		// receive initial response from the server...
		receiveDataFromServer(false);

		// Send HELO message to init...
		stringstream ss;
		ss << "HELO " << m_sSmtpServerName << "\r\n";
		exchangeMsgWithServer(ss.str());

		//Send STARTTLS message...
		ss.str(string());
		ss.clear();
		ss << "STARTTLS \r\n";
		exchangeMsgWithServer(ss.str());

		// Setup SSL connection...
		ret = establishSecureConnection();
		if (ret > 0)
		{
			stringstream ss1;
			ss1 << "AUTH LOGIN " << "\r\n";
			exchangeMsgSecurelyWithServer(ss1.str());

			// Send Username
			ss1.str(string());
			ss1.clear();
			ss1<< m_sEncrUserName << " \r\n";
			exchangeMsgSecurelyWithServer(ss1.str());

			// Send Password
			ss1.str(string());
			ss1.clear();
			ss1 << m_sEncrPwd << " \r\n";
			exchangeMsgSecurelyWithServer(ss1.str());

			// Send MAIL FROM: <sender@mydomain.com>
			ss1.str(string());
			ss1.clear();

			ss1 << "MAIL FROM:<" << m_sFromAddr << ">\r\n";
			exchangeMsgSecurelyWithServer(ss1.str());

			// Send RCPT TO: <receiver@domain.com>
			ss1.str(string());
			ss1.clear();
			ss1 << "RCPT TO:<" << m_sToAddr << ">\r\n";
			exchangeMsgSecurelyWithServer(ss1.str());

			// Send DATA
			ss1.str(string());
			ss1.clear();
			ss1 << "DATA\r\n";
			exchangeMsgSecurelyWithServer(ss1.str());

			// Send the content in the MainContentFile...
			cout << "Sending Email Content..." << endl;
			sendContentWithAttachment();

			// Send blank line and a period
			ss1.str(string());
			ss1.clear();
			ss1 << "\r\n.\r\n";
			exchangeMsgSecurelyWithServer(ss1.str());

			// Send QUIT
			ss1.str(string());
			ss1.clear();
			ss1 << "QUIT\r\n";
			exchangeMsgSecurelyWithServer(ss1.str());
		}

		// Close server socket and prepare to exit.
		cout << "Closing connection" << endl;

#ifdef _WIN32
		closesocket(m_socket);
#else
		close(m_socket);
#endif

#ifdef _WIN32
		WSACleanup();
#endif
		cout << "Bye!" << endl;
	}
	return 1;
}

void EmailSender::sendContentWithoutAttachment()
{
	stringstream ss;
	ss << "From: " << m_sFromAddr << "\r\n";
	ss << "To: " << m_sToAddr << "\r\n";
	ss << "Subject: " << m_sSubject << "\r\n";
	ss << "Content-Type:text/html;charset=\"UTF-8\"" << "\r\n";
	
	string line;
	ifstream pfile("Content.html");
	if (pfile.is_open())
	{
		while (getline(pfile, line))
		{
			ss << line << "\r\n\r\n";
		}
	}
	else
	{
		cout << "Could not locate content file!" << endl;
	}

	SSL_write(m_sslHandle, ss.str().c_str(), ss.str().length());
}

void EmailSender::sendContentWithAttachment()
{
	stringstream ss;
	ss << "From: " << m_sFromAddr << "\r\n";
	ss << "To: " << m_sToAddr << "\r\n";
	ss << "Subject: " << m_sSubject << "\r\n";
	ss << "Content-Type: multipart/mixed; boundary=\"_ABCDE_\"" << "\r\n";
	ss << "\r\n--_ABCDE_" << "\r\n";
	ss << "Content-Type:multipart/alternative;boundary=\"_FGHIJ_\"" << "\r\n";
	ss << "--_FGHIJ_" << "\r\n";
	ss << "Content-Transfer-Encoding:quoted-printable" << "\r\n";
	ss << "Content-Type:text/html;charset=\"UTF-8\"" << "\r\n\r\n";
	
	unsigned int fileSize = 0;
	char* buffer = getContent(m_sMainContentFile, fileSize);
	if (buffer != NULL)
		ss << buffer << "\r\n";
	else
		ss << "\r\n";
	delete buffer;

	string sAttachmentName = m_sAttachmentFile.substr(m_sAttachmentFile.find_last_of("\\")+1);

	ss << "--_FGHIJ_--" << "\r\n\r\n";
	ss << "--_ABCDE_" << "\r\n";
	ss << "Content-Type:text/html;name=\"" << sAttachmentName << "\"\r\n";
	ss << "Content-Transfer-Encoding:base64" << "\r\n";
	ss << "Content-Disposition:attachment;filename=\"" << sAttachmentName << "\"\r\n\r\n";
	
	buffer = getContent(m_sAttachmentFile, fileSize);
	if (buffer != NULL)
		ss << base64Encode(buffer, fileSize) << "\r\n";
	else
		ss << "\r\n";
	delete buffer;

	ss << "\r\n--_ABCDE_--";

	SSL_write(m_sslHandle, ss.str().c_str(), ss.str().length());
}

char* EmailSender::getContent(string sFilePath, unsigned int &fileSize)
{
	char* buffer = NULL;

	ifstream infile1(sFilePath.c_str());
	if (infile1.is_open())
	{
		// get size of file
		infile1.seekg(0, infile1.end);
		long size = infile1.tellg();
		infile1.seekg(0);

		fileSize = size;

		// allocate memory for file content
		buffer = new char[size];
		memset(buffer, 0, size);

		// read content of infile
		infile1.read(buffer, size);
	}
	else
	{
		cout << "Could not locate content file!" << endl;
	}
	return buffer;
}

EmailSender::~EmailSender()
{

}
