/**
 * copyright 2004 Jean-Christophe Hoelt
 *
 * This program is distributed under the terms of the
 * GNU General Public Licence
 */

#include "ios_fc.h"
#include <sys/types.h>
//#include <unistd.h> TO MAKE IT WORK ON WINDOWS : !!! WRITE NOT IMPLEMENTED !!!
#include <stdio.h>

namespace ios_fc {

FileInputStream::FileInputStream (const String path, const bool istext) {
	const char *flags = istext?"rt":"r";
	file = (void*)fopen(path,flags);
	if (!file)
		throw new Exception ("Unable to open the file");
	//fd = fileno((FILE*)file);
}

FileInputStream::~FileInputStream () {
	fclose ((FILE*)file);
}

int FileInputStream::streamAvailable() {
	return feof ((FILE*)file);
}

int FileInputStream::streamRead(VoidBuffer buffer) {
	size_t s = fread (buffer, 1, buffer.size(), (FILE*)file);
	if (s<0)
		throw new Exception ("File Read error");
	return s;
}

FileOutputStream::FileOutputStream (const String path, const bool istext) {
	const char *flags = istext?"wt":"w";
	file = (void*)fopen(path,flags);
	if (!file)
		throw new Exception ("Unable to open the file");
	//fd = fileno((FILE*)file);
}

FileOutputStream::~FileOutputStream() {
	fclose ((FILE*)file);
}

int FileOutputStream::streamWrite(VoidBuffer buffer) {
/*	ssize_t s = write (fd, buffer, buffer.size());
	if (s<0)
		throw new ios::Exception ("File Read error");
	return s;*/
	return 0;
}

};
