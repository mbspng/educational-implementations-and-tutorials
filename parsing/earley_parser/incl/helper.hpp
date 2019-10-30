/**
 * @file helper.hpp
 * helper functions and objects
 *
 * Matthias Bisping
 *
 * compilers: - clang-700.0.72
 *            - clang 3.5.2 / 3.5.0
 *            - GCC 5.2.0 / 4.8.3
 *            - Microsoft C/C++ 19.00.23026 for x86
 */

#ifndef __HELPER__HPP
#define __HELPER__HPP

#include "declarations.hpp"

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#ifdef UNIXLIKE
#include <sys/ioctl.h>
#include <unistd.h>
#ifdef __ANDROID__
#include <termios.h>
#endif
#endif
#ifdef _WIN32
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#define NOMINMAX
#endif
#include <windows.h>
#include <io.h>
#endif

#include "declarations.hpp"
#include "utf8/utf8.h"
#include "color.hpp"

namespace helper
{
using namespace COLOR;
typedef COLOR::ColorString                                                   CS;

namespace detail
{
////////////////////////////////////////////////////////////////////////////////
template<typename T>
sstr to_string(const T& s)
{
    std::stringstream ss;
    ss << s;
    return ss.str();
}
////////////////////////////////////////////////////////////////////////////////
template<typename T>
sstr to_string(const std::vector<T>& v)
{
    sstr str;
    for(auto i = v.begin(); i != v.end(); ++i) str+=*i;
    return str;
}
////////////////////////////////////////////////////////////////////////////////
} // detail
} // helper


namespace helper
{
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief tokenizes sequence \p seq by delimiter \p d and returns
 *        std::vector& of token vector
 */
template<typename T>
svec_s tokenise(T seq, const char d=' ')
{
    sstr tok;  // current token
    svec_s toks; // vector of tokens to be returned

    // convert seq to string and concat delimiter
    seq = detail::to_string(seq)+d;

    // iterate over sequence. If delimiter encountered, append token
    // to toks, else append glyph to tok
    for(auto i = seq.begin(); i != seq.end(); ++i)
    {
        if(*i != d)
        {
            tok+=*i;
        }
        else if(tok.begin() != tok.end())
        {
            toks.push_back(tok);
            tok.clear();
        }
    }
    toks.shrink_to_fit();
    return toks;
}
////////////////////////////////////////////////////////////////////////////////
/// combines hashes by taking one hash as the seed for the hashing of
/// another object
template <class T>
size_t hash_combine(size_t seed, const T& v)
{
    std::hash<T> hasher;
    return (seed ^= (hasher(v)<<16) + 0x9e3779b9 + (seed<<6) + (seed>>2));
}
////////////////////////////////////////////////////////////////////////////////
/// hashes strings
size_t hash_string(sstr s)
{
    size_t hash = 0;

    for(auto c = s.begin(); c != s.end(); ++c)
    {
        hash += *c;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}
////////////////////////////////////////////////////////////////////////////////
/// returns glyph count of utf8 conformant string
unsigned short utf8_size(sstr s)
{
    return utf8::distance(s.begin(), s.end());
}
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief sends message to specified stream
 * @param lbl label e.g. "Error: "
 * @param msg message the message body
 * @param f file the error messages comes from
 * @param ln line the error message (not the error!) was issued from
 * @param c color the message body
 * @param o the ostream to send to (default std::cerr)
 */
void msg(const sstr& lbl, const sstr& msg,
         sstr f="", int ln=-1,
         sost& o=std::cerr, CID c=RED)
{
    sstr lstr;
    if (ln > -1)
    {
        std::stringstream ss;
        ss << ln;
        ss >> lstr;
    }

    if (f.size() > 0)
    {
        f = f+":";
        if (ln > -1)
        {
            f = f+lstr; lstr="";
        }
    }
    #ifdef UNIXLIKE
        CS _lbl(lbl, c);
        CS _msg(msg);
        CS _ln(lstr);
        CS _fl(f);
    #else
        sstr _lbl(lbl);
        sstr _msg(msg);
        sstr _ln(lstr);
        sstr _fl(f);
    #endif
    o << _fl << _ln;
    if (_fl.size() > 0 || _ln.size() > 0) o << ": ";
    o << _lbl << " " << _msg << "\n";
    o.flush();
}
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Converts v of type std::vector<T> to a std::string
 * @param v vector to convert
 * @pre requires objects of type T to be concatenable with std::string
 */
template <typename T>
sstr to_string(const std::vector<T>& v)
{
    sstr str;
    for(auto t = v.begin(); t != v.end(); ++t)
    {
        str+=*t;
        if (t+1 != v.end()) str+=" ";
    }
    return str;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Converts s of type T to std::string
 * @param t object to convert
 * @pre requires t to have operator<< defined
 */
template<typename T>
sstr to_string(const T& t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}
////////////////////////////////////////////////////////////////////////////////
/// returns number of columns of current CLI window (terminal or cmd)
unsigned get_terminal_columns()
{
    unsigned c = -1;
    #ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    c = csbi.srWindow.Right - csbi.srWindow.Left ;
    #elif defined UNIXLIKE
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    c = w.ws_col;
    #else
    #error "Unkown platform"
    #endif
    return c;
}
////////////////////////////////////////////////////////////////////////////////
//                                  INCREMENT                                 //
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief handle for variable to pass to generic functions for controlling
 * the function from the outside. If the function takes an argument that it
 * applies operator++ to, this handle redirects the ++ operation to its
 * own member function.
 */
template<typename T>
class Increment
{

public:
    Increment(T& v, T i=1)
    {
        increment = i;
        value = &v;
    }

    /// adds whatever has been set as the increment to the variable the
    /// value-pointer has been set to
    T operator++()
    {
        *value = *value + (increment);
        return *value;
    }

    T* value;
    T increment;
};
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief initializes variable by looking up the argument string
 *        \p s in the file \p cf
 * @param s string to lookup associated value of
 * @param cf file to look up string in
 * @pre requires entries of the form key=value with one entry
 *      per line.
 * @param sep the character separating key from value in @p cf
 * @param cmt comment tag
 */
sstr init(sstr s, sstr cf="config.txt", char sep='=', char cmt='#')
{
    std::ifstream conf(cf.c_str());
    if (!conf)
    {
        msg("error:", "unable to open file '"+cf+"'",
            __FILE__, __LINE__);
        exit(1);
    }
    sstr line;
    while(std::getline(conf, line))
    {
        if (line.size() == 0) continue;
        svec_s tokens = tokenise(line, sep);
        if (tokens.end() - tokens.begin() > 0)
        {
            if (*(tokens[0].begin()) == cmt) continue;
        }

        if (tokens.end() - tokens.begin() != 2)
        {
            sstr tokstr = to_string(tokens);
            msg("error:", "invalid definition '"+tokstr+"' in '"+cf+"'",
                __FILE__, __LINE__);
            exit(1);
        }
        if (tokens[0] == s)
        {
            return tokens[1];
        }
    }
    msg("error:", "key '"+s+"' not found in '"+cf+"'",
        __FILE__, __LINE__);
    exit(1);
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
/// @returns number of digits of a given number in base 10
template <typename T>
int get_digits(T number)
{
    int digits = 0;
    if (number < 0) digits = 1;
    while (number) {
        number /= 10;
        digits++;
    }
    return digits;
}
////////////////////////////////////////////////////////////////////////////////
    /// fills line with @p c
    void fill_line(char c, sost&o=std::cout, int cls = 60)
    {
        // if stdout is a file, only fill part of the line
        #ifdef _WIN32
        if (_isatty(_fileno(stdout))) cls = get_terminal_columns();
        #else
        if (isatty(fileno(stdout))) cls = get_terminal_columns();
        #endif
        for (int i = 0; i < cls; ++i) o << c;
        o << '\n';
    }
////////////////////////////////////////////////////////////////////////////////
} // helper
#endif //__HELPER__HPP
