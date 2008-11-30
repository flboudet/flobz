/**
 * copyright 2004 Jean-Christophe Hoelt
 *
 * This program is distributed under the terms of the
 * GNU General Public Licence
 */

#include "ios_stream.h"
#include <iostream>

namespace ios_fc {

int InputStream::streamRead(VoidBuffer buffer) {
    return streamRead(buffer, buffer.size());
}

void InputStream::streamReadAll (VoidBuffer buf) {
	static const int step = 256;
	int nb = step;
	int offset = 0;
	buf.realloc(step);
	while (nb) {
		nb = streamRead(buf + offset);
		if (nb) {
			offset += nb;
			buf.grow (nb);
		}
	}
	buf.reduce(step);
}

int OutputStream::streamWrite(VoidBuffer buffer)
{
    return streamWrite(buffer, buffer.size());
}

}
