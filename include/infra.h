#ifndef __INFRA_H__
#define __INFRA_H__

#include <cstdio>
#include <cstring>
#include <ctime>
#include <regex>
#include <string>
#include <vector>

#include "include/info.h"


#define _str(x) #x
/**
 * \def xstr stringify
 */
#define xstr(x) _str(x)


#define ___PASTA(a, b) a##b
/**
 * \def __PASTA - join two parameters
 */
#define __PASTA(a, b) ___PASTA(a, b)

#ifdef __GNUC__
#define __UNIQUE_ID(prefix) __PASTA(__PASTA(__UNIQUE_ID, prefix), __COUNTER__)
#else
#define __UNIQUE_ID(prefix) __PASTA(__PASTA(__UNIQUE_ID, _), prefix)
#endif

#define _foreach(vec, id, _size) \
    for (unsigned int id = 0, _size = (vec).size(); id < _size; ++id)
#define foreach(vec, id) _foreach(vec, id, __UNIQUE_ID(size))


#define _iter_range(vec, id, _start, _end, _end_id) \
    for (unsigned int id = (_start), _end_id = (_end); id < _end_id; ++id)
#define iter_range(vec, id, start, end) \
    _iter_range(vec, id, start, end, __UNIQUE_ID(_end))

#define ARRAY_SIZE(arr, type) sizeof((arr)) / sizeof(type)

/**
 * split - split the @b text by using @b delimiter
 * @param text
 * @param delimiter
 * @return
 */
std::vector<std::string> split(char *text, char delimiter);

std::string join(std::vector<std::string> strings, std::string delimiter);

template <class T>
std::vector<T> operator+(std::vector<T> &op1, std::vector<T> op2)
{
    std::vector<T> result(op1.begin(), op1.end());
    foreach (op2, i) {
        result.push_back(op2[i]);
    }
    return result;
}

template <class T>
std::vector<T> operator+=(std::vector<T> &op1, std::vector<T> op2)
{
    foreach (op2, i) {
        op1.push_back(op2[i]);
    }
    return op1;
}

void stringToLower(char *text);
void stringToUpper(char *text);


struct __info_t stringToInfo(std::string s);


class isNumeric
{
private:
    static std::regex _regex;

public:
    bool operator()(std::string text);
};

void random(double *genes, int size);

int randomRange(int start, int end, int different_num);

double randomDouble();


#define _average(array, size)           \
    ({                                  \
        double sum = 0;                 \
        __typeof__(size) SIZE = (size); \
        __typeof__(SIZE) i = 0;         \
        for (i; i < SIZE; ++i) {        \
            sum += array[i];            \
        }                               \
        sum / SIZE;                     \
    })

template <class T>
double __average(T array, int size)
{
    double sum = 0;
    for (int i = 0; i < size; ++i)
        sum += array[i];
    return sum / size;
}


#ifdef WIN32
#define average __average
#else
#define average _average
#endif

#endif

#define set_bit(num, i) ((num) |= (1 << (i)))

#define check_bit(num, i) (((num) & (1 << (i))) ? 1 : 0)
