/* Ultimate Othello 1678
 * Copyright (C) 2002  Florent Boudet <flobo@ifrance.com>
 * iOS Software <http://ios.free.fr>
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

#include "IosVector.h"
#include "IosException.h"
#include <stdio.h>
#include <stdlib.h>

IosVector::IosVector()
{
	vectorData = (void **) malloc(sizeof(void *) * vectorInitialSize);
	vectorCapacity = vectorInitialSize;
	vectorSize = 0;
}

IosVector::IosVector(int size)
{
	vectorData = (void **) malloc(sizeof(void *) * size);
	vectorCapacity = size;
	vectorSize = 0;
}


IosVector::~IosVector()
{
}

void IosVector::addElement(void *element)
{
	if (vectorSize >= vectorCapacity)
		increaseVectorSize();
	vectorData[vectorSize++] = element;
}

void * IosVector::getElementAt(const int index) const
{
	if (index >= vectorSize)
		throw new Exception("Vector index out of bounds");
	return vectorData[index];
}

void IosVector::removeElementAt(const int index)
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

void IosVector::removeElement(void *element)
{
	for (int i = 0 ; i < vectorSize ; i++) {
		if (vectorData[i] == element) {
			removeElementAt(i--);
		}
	}
}

void IosVector::removeAllElements()
{
	vectorSize = 0;
}

int IosVector::getSize() const
{
	return vectorSize;
}

int IosVector::getCapacity() const
{
	return vectorCapacity;
}

void IosVector::increaseVectorSize()
{
	vectorData = (void **) realloc(vectorData, sizeof(void *) * (vectorSize + vectorSizeIncrement));
	vectorCapacity += vectorSizeIncrement;
}

void IosVector::dumpVector() const {
	fprintf(stderr, "Size: %d\n", getSize());
	for (int i = 0, j = getSize() ; i < j ; i++)
		fprintf(stderr, "elt[%d]=%d ", i, (int)getElementAt(i));
	fprintf(stderr, "\n");
}
