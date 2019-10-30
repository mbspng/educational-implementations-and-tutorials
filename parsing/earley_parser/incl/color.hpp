/**
 * @file color.hpp
 * class for wrapping color codes and easy printing
 *
 * Matthias Bisping
 *
 * compilers: - clang-700.0.72
 *            - clang 3.5.2 / 3.5.0
 *            - GCC 5.2.0 / 4.8.3
 *            - Microsoft C/C++ 19.00.23026 for x86
 */

#ifndef __COLOR__HPP
#define __COLOR__HPP

#include "declarations.hpp"

namespace COLOR
{

// color IDs
enum CID
{
    DEF = 39,
    RED = 31,
    GREEN = 32,
    BLUE = 34,
    MAGENTA = 35,
    YELLOW = 33,
    CYAN = 36,
    BLACK = 30
};
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                 ColorString                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief struct bundling a string, an ANSI color code and an ANSI bold
 *        face code
 * @param s string
 * @param c color ID
 */
struct ColorString
{
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief constructs a ColorString with color @p c and the string
     *        @p s.
     * @param s string to wrap
     * @param c color to print @p s in
     * @param bold determines whether @p s will be printed bold
     */
    ColorString(sstr s, CID c=DEF, bool bold=1)
    :str(s),
    color(c),
    bold(bold)
    {
    }
////////////////////////////////////////////////////////////////////////////////
    /// sends \b str in color of \b color to stream @p o
    friend sost& operator<<(sost& o, const ColorString& c)
    {
        o << "\033[" << c.bold << ";"
          << c.color << "m" << c.str << "\033[0m";
        return o;
    }
////////////////////////////////////////////////////////////////////////////////
    /// conversion operator to string, returns field \p str
    operator std::string() const
    {
        return str;
    }
////////////////////////////////////////////////////////////////////////////////
    /// @return size of string field
    int size()
    {
        return str.size();
    }
////////////////////////////////////////////////////////////////////////////////
    sstr str;  ///< the actual string
    CID color; ///< determines the color modifier for the string \p str
    bool bold; ///< determines whether \p str will be modified to bold
////////////////////////////////////////////////////////////////////////////////
};


}

#endif // __COLOR__HPP
