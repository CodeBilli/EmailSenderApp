# EmailSenderApp
This project provides source code for C++ based Email sending application. Currently written for Windows. Will be extended to Linux shortly.

Following are some of the features of this application:

1. Supports all modern Email servers which have TLS support.
2. Fully configurable through a property file.
3. Supports attachment of any document type.
4. The main email content can be provided as a text file. 

## How to Use

1. The main code is available in 2 files - EmailSender.cpp and EmailSender.h. smtp.cpp attached here has the main method. 
2. EmailSender.cpp and .h can be attached to any C++ application. Steps to invoke EmailSender can be found in smtp.cpp's main method.
3. File Params.txt attached shows all the input properties.
4. Attachment.html is the file to be attached with the email.
5. content.txt is the main email content.
6. Params.txt should be visible to the application running the code. Path to content.txt and Attachment.html can be provided through Params.txt
        
