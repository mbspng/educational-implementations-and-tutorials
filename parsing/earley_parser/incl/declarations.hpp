/**
 * @file declarations.hpp
 * central place for typedefs etc.
 *
 * Matthias Bisping
 *
 * compilers: - clang-700.0.72
 *            - clang 3.5.2 / 3.5.0
 *            - GCC 5.2.0 / 4.8.3
 *            - Microsoft C/C++ 19.00.23026 for x86
 */

#ifndef __DECLARATIONS__HPP
#define __DECLARATIONS__HPP

#include <iostream>
#include <ostream>
#include <vector>
#include <string>

namespace Declarations
{
    typedef std::ostream sost;
    typedef std::string sstr;
    typedef std::vector<std::string> svec_s;

    #if defined __linux || defined __APPLE__
    #define UNIXLIKE
    #endif

}

namespace Earley
{
    using namespace Declarations;
}

namespace BUSY
{
    using namespace Declarations;
}

namespace LOAD
{
    using namespace Declarations;
}

namespace COLOR
{
    using namespace Declarations;
}

namespace helper
{
    using namespace Declarations;
}

namespace std
{
    using namespace Declarations;
}

#endif //__DECLARATIONS__HPP
