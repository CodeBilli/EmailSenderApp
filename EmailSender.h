#pragma once
class EmailSender
{
private:
	int m_socket;
	SSL *m_sslHandle;
	string m_sSmtpServerName;
	string m_sFromAddr;
	string m_sToAddr;
	string m_sMsgFile;
	string m_sEncrUserName;
	string m_sEncrPwd;
	string m_sSubject;
	string m_sMainContentFile;
	string m_sAttachmentFile;

	int connectToSMTPServer();
	int exchangeMsgWithServer(string message);
	int exchangeMsgSecurelyWithServer(string message);
	int establishSecureConnection();
	int receiveDataFromServer(bool bSecured);
	vector<string> tokenize(string str, string delimiters);
	void sendContentWithoutAttachment();
	void sendContentWithAttachment();
	string base64Encode(char const* bytes_to_encode, unsigned int in_len);
	char* getContent(string sFilePath, unsigned int &fileSize);
public:
	EmailSender();
	bool parseInputFile();
	int sendEmail();
	~EmailSender();
};

