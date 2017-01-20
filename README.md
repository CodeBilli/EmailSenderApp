# EmailSenderApp
This project provides source code for C++ based Email sending application. Supports both Windows and Unix.

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
5. Content.html is the main email content. This can be a txt file as well.
6. Params.txt should be visible to the application running the code. Path to Content.html and Attachment.html can be provided through Params.txt

## Building steps

In Unix,

make clean
make smtp

In Windows, 
create a project and include the files available here. Make sure openssl is available.
        
