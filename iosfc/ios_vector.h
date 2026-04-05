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

#ifndef _IOS_VECTOR_H
#define _IOS_VECTOR_H

#include <vector>
#include <memory>

namespace ios_fc {

template <typename T>
class Vector {
public:
    Vector() = default;
    Vector(const Vector<T> &other) = default;
    Vector<T> &operator=(const Vector<T> &other) = default;

    void add(T *element) { data.push_back(element); }
    int size() const { return static_cast<int>(data.size()); }
    T *&operator[](int index) { return data[index]; }
    T *const &operator[](int index) const { return data[index]; }
    T *&get(int index) { return data[index]; }
    T *const &get(int index) const { return data[index]; }

    int remove(const T *element) {
        for (int i = static_cast<int>(data.size()) - 1; i >= 0; --i) {
            if (data[i] == element) {
                data.erase(data.begin() + i);
                return i;
            }
        }
        return -1;
    }

    void removeAt(int index) {
        data.erase(data.begin() + index);
    }

    void removeAtKeepOrder(int index) {
        data.erase(data.begin() + index);
    }

    void clear() { data.clear(); }

    Vector<T> dup() const {
        Vector<T> buf;
        buf.data = data;
        return buf;
    }

    typename std::vector<T *>::iterator begin() { return data.begin(); }
    typename std::vector<T *>::const_iterator begin() const { return data.begin(); }
    typename std::vector<T *>::iterator end() { return data.end(); }
    typename std::vector<T *>::const_iterator end() const { return data.end(); }

private:
    std::vector<T *> data;
};

template <typename T>
class SelfVector {
public:
    SelfVector() = default;
    SelfVector(const SelfVector<T> &other) = default;
    SelfVector<T> &operator=(const SelfVector<T> &other) = default;

    void add(const T &element) { data.emplace_back(std::make_shared<T>(element)); }
    void add(T *element) { data.emplace_back(element); }

    int size() const { return static_cast<int>(data.size()); }

    T &operator[](int index) { return *data[index]; }
    const T &operator[](int index) const { return *data[index]; }

    int remove(const T *element) {
        for (int i = static_cast<int>(data.size()) - 1; i >= 0; --i) {
            if (data[i].get() == element) {
                data.erase(data.begin() + i);
                return i;
            }
        }
        return -1;
    }

    void remove() {
        if (!data.empty()) {
            data.pop_back();
        }
    }

    void removeAt(int index) {
        data.erase(data.begin() + index);
    }

    void removeAtKeepOrder(int index) {
        data.erase(data.begin() + index);
    }

    typename std::vector<std::shared_ptr<T>>::iterator begin() { return data.begin(); }
    typename std::vector<std::shared_ptr<T>>::const_iterator begin() const { return data.begin(); }
    typename std::vector<std::shared_ptr<T>>::iterator end() { return data.end(); }
    typename std::vector<std::shared_ptr<T>>::const_iterator end() const { return data.end(); }

private:
    std::vector<std::shared_ptr<T>> data;
};

}

#endif // _IOS_VECTOR_H
