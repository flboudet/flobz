/* Ultimate Othello 1678
 * Copyright (C) 2002  Florent Boudet <flobo@ifrance.com>
 * iOS Software <http://www.ios-software.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 */

#include "ios_vector.h"
#include "ios_exception.h"
#include <stdio.h>
#include <stdlib.h>

namespace ios_fc {

Vector::Vector()
{
	vectorData = (void **) malloc(sizeof(void *) * vectorInitialSize);
	vectorCapacity = vectorInitialSize;
	vectorSize = 0;
}

Vector::Vector(int size)
{
	vectorData = (void **) malloc(sizeof(void *) * size);
	vectorCapacity = size;
	vectorSize = 0;
}


Vector::~Vector()
{
}

void Vector::addElement(void *element)
{
	if (vectorSize >= vectorCapacity)
		increaseVectorSize();
	vectorData[vectorSize++] = element;
}

void * Vector::getElementAt(const int index) const
{
	if (index >= vectorSize)
		throw new Exception("Vector index out of bounds");
	return vectorData[index];
}

void Vector::removeElementAt(const int index)
{
	if (index >= vectorSize)
		throw new Exception("Vector index out of bounds");
	vectorSize--;
	if (index == vectorSize)
		return;
	for (int i = index ; i < vectorSize ; i++) {
		vectorData[i] = vectorData[i+1];
	}
}

void Vector::removeElement(void *element)
{
	for (int i = 0 ; i < vectorSize ; i++) {
		if (vectorData[i] == element) {
			removeElementAt(i--);
		}
	}
}

void Vector::removeAllElements()
{
	vectorSize = 0;
}

int Vector::getSize() const
{
	return vectorSize;
}

int Vector::getCapacity() const
{
	return vectorCapacity;
}

void Vector::increaseVectorSize()
{
	vectorData = (void **) realloc(vectorData, sizeof(void *) * (vectorSize + vectorSizeIncrement));
	vectorCapacity += vectorSizeIncrement;
}

void Vector::dumpVector() const {
	fprintf(stderr, "Size: %d\n", getSize());
	for (int i = 0, j = getSize() ; i < j ; i++)
		fprintf(stderr, "elt[%d]=%d ", i, (int)getElementAt(i));
	fprintf(stderr, "\n");
}

};

