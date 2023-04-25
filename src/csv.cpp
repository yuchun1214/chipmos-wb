#include "include/csv.h"
#include <sys/types.h>
#include <clocale>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <locale>
#include <stdexcept>

#define MAX_LEN 8192

csv_t::csv_t(csv_t &csv)
{
    this->_file = NULL;
    this->_data = csv._data;
    this->_head = csv._head;
    this->_mode = csv._mode;
    this->_filename = csv._filename;
}

csv_t::csv_t()
{
    _file = NULL;
}

csv_t::csv_t(std::string filename, std::string mode)
{
    _filename = filename;
    _mode = mode;
    _file = NULL;
}

csv_t::csv_t(std::string filename,
             std::string mode,
             bool r,
             bool head,
             int r1,
             int r2)
{
    _filename = filename;
    _mode = mode;
    _file = NULL;
    bool retval;
    if (r) {
        retval = read(filename, mode, head, r1, r2);
    }
}

std::vector<std::string> csv_t::parseCsvRow(char *text, char delimiter)
{
    std::vector<std::string> data;
    std::string token;

    typedef enum {
        STATE_NextCharacter,
        STATE_ParseString,
        STATE_QuotationInString,
        STATE_EndOfLine,
        STATE_Error,
        STATE_StateCount
    } ParserState;

    ParserState state = STATE_NextCharacter;

    while (state != STATE_EndOfLine) {
        int c = *text++;
        if (c == EOF) {
            switch (state) {
            case STATE_ParseString:
                state = STATE_Error;
                break;
            case STATE_QuotationInString:
                data.push_back(token);
                token.clear();
            case STATE_NextCharacter:
                state = STATE_EndOfLine;
                break;
            case STATE_Error:
                std::cerr << "Error in csv_t::parseCsvRow()" << std::endl;
                abort();
            default:
                break;
            }
        }
        switch (state) {
        case STATE_NextCharacter:
            if (c == '\n' || c == '\r' || c == '\0') {
                data.push_back(token);
                token.clear();
                state = STATE_EndOfLine;
                break;
            } else if (c == delimiter) {
                data.push_back(token);
                token.clear();
                break;
            } else if (c == '"') {
                state = STATE_ParseString;
                break;
            }
            token.append(1, static_cast<char>(c));
            break;
        case STATE_ParseString:
            if (c == '"') {
                state = STATE_QuotationInString;
                break;
            }
            token.append(1, static_cast<char>(c));
            break;
        case STATE_QuotationInString:
            if (c == '"') {
                token.append(1, static_cast<char>(c));
                state = STATE_ParseString;
                break;
            } else if (c == delimiter) {
                data.push_back(token);
                token.clear();
                state = STATE_NextCharacter;
                break;
            } else if (c == '\n' || c == '\r' || c == '\0') {
                data.push_back(token);
                token.clear();
                state = STATE_EndOfLine;
                break;
            } else {
                /* 1. >>>1,2,st""r3<<<
                 * Escaping double quotation mark without denoting token as
                 * string
                 *
                 * 2. >>>1,2,"st"r3"<<<
                 *    Have a wild double quotation mark in a string
                 *
                 * 3. >>>1,2,"str3" ,<<<
                 *    Delimiter not located right after end of string
                 */
                state = STATE_Error;
                break;
            }
        default:
            break;
        }
    }

    return data;
}

std::string csv_t::formCsvElement(std::string text)
{
    bool hasSpecialChar = false;
    for (unsigned int i = 0, length = text.length(); i < length; ++i) {
        if (text[i] == ',') {
            hasSpecialChar = true;
        } else if (text[i] == '"') {
            hasSpecialChar = true;
            text.insert(text.begin() + i, '"');
        }
    }
    if (hasSpecialChar) {
        text.insert(text.begin(), '"');
        text.append("\"");
    }

    return text;
}

void csv_t::trim(std::string text)
{
    size_t found;
    foreach (_data, i) {
        foreach (_data[i], j) {
            found = _data[i][j].find_last_not_of(text);
            if (found != std::string::npos)
                _data[i][j].erase(found + 1);
            else
                _data[i][j].clear();
        }
    }
}


csv_t::~csv_t()
{
    if (_file != NULL)
        fclose(_file);
    _file = NULL;
}

bool csv_t::_hasBOM(char *_text, unsigned int bom, short bits)
{
    unsigned int result, *text;
    text = (unsigned int *) _text;
    result = *text ^ bom;
    return !(result << bits);
}

// bool csv_t::read(bool head, int r1, int r2)
// {
//     return read(_filename, _mode, head, r1, r2);
// }

bool csv_t::read(std::string filename,
                 std::string mode,
                 bool head,
                 int r1,
                 int r2)
{
    _data.clear();
    if (filename.compare("") == 0) {
        filename = _filename;
    }

    if (mode.compare("") == 0) {
        mode = _mode;
    }

    if (_file) {
        close();
    }

    _file = nullptr;
    _file = fopen(filename.c_str(), mode.c_str());

    if (_file == nullptr) {
        if (errno == ENOENT) {
            throw std::ios_base::failure("No such file or directory");
        } else if (errno == EINVAL) {
            std::string err = "\"" + mode + "\"is and invalid argument.";
            throw std::invalid_argument(err);
        } else {
            throw std::ios_base::failure("Error on opening file");
        }
    }

    size_t size = 0;
    char line_ptr[MAX_LEN];
    std::vector<std::string> text;

    if (fgets(line_ptr, MAX_LEN, _file)) {
        size = strlen(line_ptr);
        // check byte order mark
        if (_hasBOM(line_ptr, 0x00BFBBEF,
                    8)) {  // EF BB BF, 0x00BFBBEF for little endian
            memmove(line_ptr, line_ptr + 3, size - 3);  // remove byte order
        } else if (_hasBOM(line_ptr, 0x0000FFFE,
                           16)) {  // FE FF, 0x0000FFFE for little endian
            memmove(line_ptr, line_ptr + 2, size - 2);  // remove byte order
        }
    } else {
        return false;
    }


    text = parseCsvRow(line_ptr, ',');
    if (head) {
        for (int i = 0, size = text.size(); i < size; ++i) {
            if (text[i].compare("") != 0) {
                _head[text[i]] = i;
            } else {
                throw std::invalid_argument(
                    "Column head contains empty head, failed to set the "
                    "header");
                // return false;  // col head has empty head fail to set head
            }
        }
    } else {
        _data.push_back(text);
    }

    while (fgets(line_ptr, MAX_LEN, _file)) {
        text = parseCsvRow(line_ptr, ',');
        _data.push_back(text);
    }

    if (r1 >= 0 && r2 < 0) {
        _data.erase(_data.begin() + r1, _data.end());
    } else if (r1 >= 0 && r2 >= 0) {
        _data.erase(_data.begin() + r2, _data.end());
        _data.erase(_data.begin(), _data.begin() + r1);
    }

    return true;
}

void csv_t::setMode(std::string mode)
{
    _mode = mode;
}

void csv_t::setFileName(std::string filename)
{
    _filename = filename;
}

void csv_t::setHeader(std::string old, std::string neun, bool replace)
{
    _head[neun] = _head[old];
    if (replace) {
        _head.erase(old);
    }
}

void csv_t::setHeaders(std::map<std::string, std::uint16_t> head)
{
    _head = head;
}

void csv_t::setHeaders(std::map<std::string, std::string> mapping, bool replace)
{
    for (std::map<std::string, std::string>::iterator it = mapping.begin();
         it != mapping.end(); ++it) {
        setHeader(it->second, it->first, replace);
    }
}

void csv_t::close()
{
    if (_file) {
        fclose(_file);
        _file = NULL;
    }
}

void csv_t::addData(std::map<std::string, std::string> elements)
{
    std::vector<std::string> data;
    data.reserve(elements.size());
    std::map<std::string, std::uint16_t> head;

    // if _head is empty, set the header
    int idx = 0;
    if (_head.empty()) {
        for (std::map<std::string, std::string>::iterator it = elements.begin();
             it != elements.end(); ++it) {
            head[it->first] = idx;
            ++idx;
        }
        setHeaders(head);
    }



    for (std::map<std::string, std::string>::iterator it = elements.begin();
         it != elements.end(); ++it) {
        try {
            idx = _head.at(it->first);
        } catch (std::out_of_range &e) {
            std::string err_msg = e.what();
            err_msg += " There is no such head " + it->first +
                       " in the exist dataframe";
            throw std::out_of_range(err_msg);
        }
        data.insert(data.begin() + idx, it->second);
    }
    _data.push_back(data);
}

bool csv_t::write(std::string filename, std::string mode, bool head)
{
    if (filename.length() == 0)
        filename = _filename;

    if (mode.length() == 0)
        mode = _mode;

    if (_file) {
        close();
    }
    _file = fopen(filename.c_str(), mode.c_str());
    int retval = 0;
    std::vector<std::string> strings_temp;
    std::string row;
    if (head) {
        for (std::map<std::string, std::uint16_t>::iterator it = _head.begin();
             it != _head.end(); it++) {
            strings_temp.push_back(it->first);
        }
        row = join(strings_temp, ",");
        retval = fprintf(_file, "%s\n", row.c_str());
        if (retval < 0) {
            return false;
        }
    }
    foreach (_data, i) {
        strings_temp.clear();
        for (std::map<std::string, std::uint16_t>::iterator it = _head.begin();
             it != _head.end(); ++it) {
            strings_temp.push_back(formCsvElement(_data[i][it->second]));
        }
        row = join(strings_temp, ",");
        retval = fprintf(_file, "%s\n", row.c_str());
        if (retval < 0) {
            return false;
        }
    }

    return true;
}


std::vector<std::string> csv_t::getRow(int row)
{
    int idx;
    if (row < 0) {
        idx = (int) _data.size() + row;
    } else
        idx = row;
    try {
        return _data.at(idx);
    } catch (std::out_of_range &e) {
        std::string err_msg = e.what();
        err_msg += std::to_string(row) +
                   "is out of range"
                   " in the exist dataframe";
        throw std::out_of_range(err_msg);
    }
}

std::string csv_t::getElement(std::string _col, int row)
{
    uint16_t col = _head[_col];
    try {
        return getRow(row).at(col);
    } catch (std::out_of_range &e) {
        std::string err_msg = e.what();
        err_msg += std::to_string(col) +
                   "is out of range"
                   " in the exist dataframe. The exception is thrown "
                   "in the function getElement(std::string, int)";
        throw std::out_of_range(err_msg);
    }
}

std::string csv_t::getElement(int col, int row)
{
    try {
        return getRow(row).at(col);
    } catch (std::out_of_range &e) {
        std::string err_msg = e.what();
        err_msg += std::to_string(col) +
                   "is out of range"
                   " in the exist dataframe. The exception is thrown "
                   "in the function getElement(int, int)";
        throw std::out_of_range(err_msg);
    }
}

std::map<std::string, std::string> csv_t::getElements(int row)
{
    std::map<std::string, std::string> data;

    for (std::map<std::string, std::uint16_t>::iterator it = _head.begin();
         it != _head.end(); ++it) {
        data[it->first] = _data[row][it->second];
    }

    return data;
}

unsigned int csv_t::nrows()
{
    return _data.size();
}

std::vector<std::vector<std::string> > csv_t::getData(int r1, int r2)
{
    std::vector<std::vector<std::string> > data;
    if ((r1 < 0 && r2 > 0) || (r1 > 0 && r2 < -1)) {
        throw std::invalid_argument("given r1 < 0 but r2 >0");
    } else if (r1 > 0 && r2 == -1) {
        r2 = _data.size();
    } else {
        return _data;
    }

    iter_range(_data, i, (unsigned int) r1, (unsigned int) r2)
    {
        data.push_back(_data[i]);
    }
    return data;
}

csv_t csv_t::filter(std::string head, std::string value)
{
    csv_t newcsv;

    std::vector<std::vector<std::string> > data;
    int idx = _head[head];
    foreach (_data, i) {
        if (_data[i][idx].compare(value) == 0) {
            data.push_back(_data[i]);
        }
    }

    newcsv._data = data;
    newcsv._filename = _filename;
    newcsv._head = _head;
    newcsv._mode = _mode;

    return newcsv;
}

csv_t csv_t::filter(std::string head, std::string value, std::string value2)
{
    csv_t newcsv;

    std::vector<std::vector<std::string> > data;
    int idx = _head[head];
    foreach (_data, i) {
        if (_data[i][idx].compare(value) == 0 ||
            _data[i][idx].compare(value2) == 0) {
            data.push_back(_data[i]);
        }
    }

    newcsv._data = data;
    newcsv._filename = _filename;
    newcsv._head = _head;
    newcsv._mode = _mode;

    return newcsv;
}

std::vector<std::string> csv_t::getColumn(std::string head)
{
    int idx = _head[head];
    std::vector<std::string> cols;

    foreach (_data, i) {
        cols.push_back(_data[i][idx]);
    }

    return cols;
}

void csv_t::dropNullRow()
{
    std::vector<std::vector<std::string> > data = _data;
    _data.clear();
    foreach (data, i) {
        if (data[i].size() == 1) {
            if (data[i][0].length() != 0) {
                _data.push_back(data[i]);
            }
        } else {
            _data.push_back(data[i]);
        }
    }
}

std::vector<std::string> csv_t::getHeader()
{
    std::vector<std::string> head(_head.size(), "");
    for (auto it = _head.begin(); it != _head.end(); ++it) {
        head[it->second] = it->first;
    }

    return head;
}
