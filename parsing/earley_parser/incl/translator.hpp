/*
 * @file translator.hpp
 * This class provides an alternative translation method to the one programmed
 * directly into Earley::Grammar<VALIDATOR, RULEPARSER>. It is used when SOVERLOAD
 * is disabled, because it has a linear time lookup for elements of type \b ES
 * but a constant lookup time (1) for elements of type \b IS. As typically there
 * are significantly more insatnces of \b ES (namely words) to map to some \b
 * instance of \b IS when SOVERLOAD is enabled (becuase an entire lexicon needs
 * to be mapped this class will not be used in these cases.
 * For smaller sets of words this class provides a faster lookup of words through
 * their ID at the cost of an increased insertion time for new words. But given
 * that the set of words for only syntactic categories is fairly small, this
 * increase in insertion time is not a real issue.
 * Primarily though it allows for reduced memory consumption, as not every pair of
 * \b IS \b ES is stored twice for the reverse mapping, which also is the source
 * of the direction dependent lookup speed.
 *
 * Matthias Bisping
 *
 * compilers: - clang-700.0.72
 *            - clang 3.5.2 / 3.5.0
 *            - GCC 5.2.0 / 4.8.3
 *            - Microsoft C/C++ 19.00.23026 for x86
 */

#ifndef __TRANSLATOR__HPP
#define __TRANSLATOR__HPP

#ifndef NDEBUG
#define NDEBUG
#endif

#include <vector>
#include <algorithm>
#include "assert.h"

/**
 * @brief translates external symbols of type \b ES to internal symbols of type
 *        \b IS
 * @details uses different types of storage for both types of symbols resulting
 * in a slow insertion and lookup time for symbols of type \b ES and constant
 * lookup time for items of type \b IS (\b IS items are not inserted by themselves)
 * @tparam INTERNSYM internal symbol type
 * @tparam EXTERNSYM external symbol type
 */
template <typename INTERNSYM, typename EXTERNSYM>
class Translator
{
////////////////////////////////////////////////////////////////////////////////
public: // TYPES
////////////////////////////////////////////////////////////////////////////////
    typedef INTERNSYM                                                        IS;
    typedef EXTERNSYM                                                        ES;
    typedef std::vector<ES>                                               ESVec;
////////////////////////////////////////////////////////////////////////////////
public: // METHODS
////////////////////////////////////////////////////////////////////////////////
    Translator() { is_entries = 0; }
////////////////////////////////////////////////////////////////////////////////
    /// adds @p es to known entries and assigns it a \b IS translation
    IS translate(const ES& es)
    {
        int i = find(es);
        if (i == -1)
        {
            es_entries.push_back(es);
            return is_entries++;
        }
        return i;
    }
////////////////////////////////////////////////////////////////////////////////
    /// looks up translation of @p is, throws error if none exists
    ES translate(const IS& is) const
    {
        if(is <= is_entries)
        {
            assert(is <= es_entries.size());
            return es_entries[is];
        }
        throw 0;
    }
////////////////////////////////////////////////////////////////////////////////
private: // METHODS
////////////////////////////////////////////////////////////////////////////////
    /// looks up index of @p es in \b es_entries, @returns -1 if @p es is unknown
    int find(ES es)
    {
        int i = 0;
        for (auto e = es_entries.begin(); e != es_entries.end(); ++e, ++i)
        {
            if (*e == es) return i;
        }
        return -1;
    }
////////////////////////////////////////////////////////////////////////////////
private: // FIELDS
////////////////////////////////////////////////////////////////////////////////
    IS is_entries;
    ESVec es_entries;
};

#endif // __TRANSLATOR__HPP

