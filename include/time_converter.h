#ifndef __TIME_CONVERTER_H__
#define __TIME_CONVERTER_H__

#include <ctime>
#include <regex>
#include <string>
#include <vector>

/**
 * @brief Abstract base class of some Regular Expression of date/time format
 */
class time_converter_base_t
{
private:
    std::string pattern_str;
    std::regex pattern;

protected:
    void initialized_tm(struct tm *_tm);
    time_converter_base_t(std::string pattern);

public:
    virtual bool isThisType(std::string text);
    virtual time_t operator()(std::string text) = 0;
};

/**
 * @brief Regular Expression with format : "YY-MM-DD HH:MM"
 */
class time_converter_with_dash_without_second_t : public time_converter_base_t
{
public:
    time_converter_with_dash_without_second_t();
    virtual time_t operator()(std::string text);
};

/**
 * @brief Regular Expression with format : "YY-MM-DD HH:MM:SS"
 */
class time_converter_with_dash_with_second_t : public time_converter_base_t
{
public:
    time_converter_with_dash_with_second_t();
    virtual time_t operator()(std::string text);
};

/**
 * @brief Regular Expression with format : "YY/MM/DD HH:MM"
 */
class time_converter_with_slash_without_second_t : public time_converter_base_t
{
public:
    time_converter_with_slash_without_second_t();
    virtual time_t operator()(std::string text);
};

/**
 * @brief Regular Expression with format : "YY/MM/DD HH:MM:SS"
 */
class time_converter_with_slash_with_second_t : public time_converter_base_t
{
public:
    time_converter_with_slash_with_second_t();
    virtual time_t operator()(std::string text);
};

/**
 * @brief Regular Expression with format : "YY/MM/DD"
 */
class time_converter_only_date_with_slash_t : public time_converter_base_t
{
public:
    time_converter_only_date_with_slash_t();
    virtual time_t operator()(std::string text);
};

/**
 * @brief Regular Expression with format : "YY-MM-DD"
 */
class time_converter_only_date_with_dash_t : public time_converter_base_t
{
public:
    time_converter_only_date_with_dash_t();
    virtual time_t operator()(std::string text);
};


/**
 * @brief timeConverter that converts date/time to time_t format, in
 * respect of base_time
 */

class timeConverter
{
private:
    static std::vector<time_converter_base_t *> converters;

    time_t _base_time;

public:
    /**
     * initialize timeConverter with base time 0
     * @brief Default constructor
     * @see timeConverter(std::string text)
     * @see timeConverter(time_t base_time)
     */
    inline timeConverter() : _base_time(0) {}
    /**
     * Construct a timeConverter object with string as base_time
     * @brief Constructor
     * @param text : string type as base_time
     */
    inline timeConverter(std::string text) : _base_time(0)
    {
        _base_time = this->operator()(text);
    }
    /**
     * Construct a timeConverter object with integer as base_time
     * @brief Constructor
     * @param text : integer type as base_time
     */
    inline timeConverter(time_t base_time) : _base_time(base_time) {}
    /**
     * @brief getter of base time
     */
    inline time_t getBaseTime() { return _base_time; }
    /**
     * @brief setter of base time
     */
    inline void setBaseTime(std::string text)
    {
        _base_time = 0;
        _base_time = this->operator()(text);
    }
    /**
     * @brief Overloads parenthesis operator
     * @param text : date/time string that matches Regular Expression
     * @return time_t value relative to base_time
     */
    time_t operator()(std::string text);

    /**
     * @brief Overloads subtraction operator
     * @param tc : a nother timeConverter object
     * @return time_t value which is the result of subtraction of two time
     */
    time_t operator-(timeConverter tc);
};

#endif
