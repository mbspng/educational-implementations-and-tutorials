/**
 * @file rule.hpp
 * Generic rule class. Rule type is determined by Grammar type. The
 * Grammar determines how the rule will be used, as it has the capability
 * to represent differnet kinds of phrase structure rules. It may be
 * used for example for CFGs, SCFGs or CSGs, as its internal storage is
 * a vector of vectors of symbols. The type of the symbols again depends
 * on the Grammar.
 * Due to the generic nature of this class, it has relatively verbose
 * access functionalitiy, that requires a more of work to be done in
 * external accessors.
 *
 * Matthias Bisping
 *
 * compilers: - clang-700.0.72
 *            - clang 3.5.2 / 3.5.0
 *            - GCC 5.2.0 / 4.8.3
 *            - Microsoft C/C++ 19.00.23026 for x86
 */

#ifndef __RULE__HPP
#define __RULE__HPP

#ifndef NDEBUG
#define NDEBUG
#endif

#include "declarations.hpp"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <ostream>
#include <stdio.h>

#include "helper.hpp"
#include "grammar.hpp"
#include "color.hpp"

namespace Earley
{
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                      Rule                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief class for representing a phrase structure rule
 * @tparam GRAMMAR Grammar type; determines behaviour of \b Rule
 */
template <typename GRAMMAR>
class Rule
{
////////////////////////////////////////////////////////////////////////////////
public:                                                     //      PUBLIC TYPES
////////////////////////////////////////////////////////////////////////////////
/// type of the host grammar
typedef GRAMMAR                                                         Grammar;
/// type of the internal symbols
typedef typename Grammar::IS                                                 IS;
/// type of the external symbols
typedef typename Grammar::ES                                                 ES;
/// vector of internal symbols
typedef typename Grammar::ISVec                                           ISVec;
/// vector of external symbols
typedef typename Grammar::ESVec                                           ESVec;
/// rule sides are of type \b ISVec
typedef ISVec                                                          Ruleside;
/// the sides of a rule are a vector of \b Ruleside objects
typedef std::vector<Ruleside>                                       RulesideVec;
////////////////////////////////////////////////////////////////////////////////
public:                                                     //    PUBLIC METHODS
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief constructs empty a \b Rule
     */
    Rule()
    :rhs_begin(-1)
    {
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief constructs a \b Rule from a a vector of sides of elements of
     *        type IS
     * @param g pointer to host grammar object; needed for translation
     *        of internal to external symbols
     * @param v vector of rule sides of type IS
     * @param b index where rhs begins, as most grammars would have
     *        2 sides, the deault index is 1
     */
    Rule(Grammar* g, RulesideVec v, short unsigned b=1)
    :rhs_begin(b)
    {
        grammar_ptr = g;
        sides.resize(v.size());
        for (int i = 0; i < b; ++i)
        {
            sides[i] = v[i];
        }
        // copy rule sides into \b sides
        for (unsigned i = b; i < v.size(); ++i)
        {
            sides[i] = v[i];
        }
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief copy constructor
     */
    Rule(const Rule& src)
    :rhs_begin(-1)
    {
        // copy rule sides into \b sides
        sides.resize(src.sides.size());
        for (unsigned i = 0; i < src.sides.size(); ++i)
        {
            sides[i] = src.sides[i];
        }
        // copy the index of where the rhs begins
        rhs_begin = src.rhs_begin;
        // copy the pointer to the host grammar
        grammar_ptr = src.grammar_ptr;
    }
////////////////////////////////////////////////////////////////////////////////
    /// @returns constant reference to rule side at index @p index
    const Ruleside& operator[](unsigned short index) const
    {
        if ((sides.end() - sides.begin()) > index)
        {
            return sides[index];
        }
        static Ruleside empty_rule;
        return empty_rule;
    }
////////////////////////////////////////////////////////////////////////////////
    /// @returns reference to rule side at index @p index
    Ruleside& operator[](unsigned short index)
    {
        if ((sides.end() - sides.begin()) > index)
        {
            return sides[index];
        }
    }
////////////////////////////////////////////////////////////////////////////////
    /// @returns const iterator to begin of sides
    typename RulesideVec::const_iterator get_lhs() const
    {
        return sides.begin();
    }
////////////////////////////////////////////////////////////////////////////////
    /// @returns const iterator to ruleside at index rhs_begin
    typename RulesideVec::const_iterator get_rhs() const
    {
        return sides.begin()+rhs_begin;
    }
////////////////////////////////////////////////////////////////////////////////
    /// @returns iterator to begin of sides
    typename RulesideVec::iterator get_lhs()
    {
        return sides.begin();
    }
////////////////////////////////////////////////////////////////////////////////
    /// @returns to ruleside at index rhs_begin
    typename RulesideVec::iterator get_rhs()
    {
        return sides.begin()+rhs_begin;
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief Sends string representation of rule @p r to argument stream @p o
     * @param o stream to send to
     * @param r rule to send representation of to @p o
     */
    friend sost& operator<<(sost& o, const Rule& r)
    {
        for(auto w=r.sides.begin()->begin();w!=r.sides.begin()->end();++w)
        {
            o << r.translate_to_es(*w) << " ";
        }
        o << r.grammar_ptr->separator;
        for (auto i = r.sides.begin()+1; i != r.sides.end(); ++i)
        {
            for(auto w = i->begin(); w != i->end(); ++w)
            {
                o << " " << r.translate_to_es(*w);
            }
            if (i+1 != r.sides.end()) o << ",";
        }
        return o;
    }
////////////////////////////////////////////////////////////////////////////////
    /// Compares rules for equality. Two rule are equal, if
    /// their sides are equal.
    bool operator==(const Rule<Grammar>& r) const
    {
        return r.sides == sides;
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief assigment operator: assignes fields from @p src to \b *this
     * @param src instance of \b Earley::Rule<GRAMMAR> to copy values from
     */
    void operator=(const Rule& src)
    {
        if (this != &src)
        {
            for (auto s = sides.begin(); s != sides.end(); ++s)
            {
                s->clear();
            }
            sides.clear();
            sides.resize(src.sides.size());
            for (short unsigned i = 0; i < src.sides.size(); ++i)
            {
                sides[i] = src.sides[i];
            }
            rhs_begin = src.rhs_begin;
        }
    }
////////////////////////////////////////////////////////////////////////////////
    /* Rule provides a couple of  interface functions for easy writing
     * of rule parser functors. They hide the implemantation for the
     * user but have familiar names, so he may easily infer what each
     * method does highlevel.
     */
    /// @returns itertaor begin() of \p sides
    typename RulesideVec::iterator begin()
    {
        return sides.begin();
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @returns iterator end() of \p sides
     */
    typename RulesideVec::iterator end()
    {
        return sides.end();
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief appends \b Ruleside @p r to \b sides
     * @param r \b Ruleside to append to \b sides
     */
    void push_back(Ruleside r)
    {
        sides.push_back(r);
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief  direct acces to sides, in case interface functions prove to be
     *         insuffiecient for a specific task
     * @return \p sides
     */
    const RulesideVec& get_sides() const
    {
        return sides;
    }
////////////////////////////////////////////////////////////////////////////////
    /// sets \p rhs_begin to @p i
    void set_rhs_begin(unsigned i)
    {
        rhs_begin = i;
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief translates instance @p es of type \b ES into an instance of type
     *        \b IS
     *        Adds an entry for @p es into \b esism if @p es is not known yet
     * @param es token to translate
     * @return \b IS translation of @p es
     */
    IS translate_to_is(const ES& es) const
    {
        return grammar_ptr->translate(es);
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief translates instance @p is of type \b IS into an instance of type
     *        \b ES
     * @param is token to translate
     * @return \b ES translation of @p is
     */
    ES translate_to_es(const IS& is) const
    {
        return grammar_ptr->translate(is);
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief parses vector of external tokens @p v into vector of sides
     *        and an rhs_begin index
     * @param v vector of tokens of type \b ES
     * @return std:pair of RulesideVec and an index for \b rhs_begin
     */
    std::pair<std::vector<ISVec>, int>  parse(ESVec v)
    {
        return grammar_ptr->parse(helper::tokenise(v));
    }
////////////////////////////////////////////////////////////////////////////////
private:                                                    //  PRIVATHE METHODS
////////////////////////////////////////////////////////////////////////////////
    /// initialises rule
    void init()
    {
        sides.clear();
    }
////////////////////////////////////////////////////////////////////////////////
private:                                                    //    PRIVATE FIELDS
////////////////////////////////////////////////////////////////////////////////
    RulesideVec sides;    ///< rule sides
    int rhs_begin;        ///< sides-index to where rhs begins
    Grammar* grammar_ptr; ///< pointer to host grammar
////////////////////////////////////////////////////////////////////////////////
}; // Rule

////////////////////////////////////////////////////////////////////////////////
//                                CFGRuleParser                               //
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  Parses vector of tokens of type EXTERNSYM into a vector of
 *         vectors of elements of type INTERNSYM. INTERNSYM will be the
 *         type, in which all dependent objects store rule tokens.
 *         EXTERNSYM should be the type of the input to the grammar.
 * @tparam INTERNSYM internal symbol type; should be integer type
 * @tparam EXTERNSYM external symbol type; should be std::string, char
 *         or char vector
 */
template <typename INTERNSYM, typename EXTERNSYM>
struct CFGRuleParser
{
////////////////////////////////////////////////////////////////////////////////
typedef INTERNSYM                                                            IS;
typedef EXTERNSYM                                                            ES;
typedef std::vector<IS>                                                   ISVec;
typedef std::vector<ES>                                                   ESVec;
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief Parses vector of tokens of type EXTERNSYM into a std::pair
     *        of a vector of vectors of elements of type EXTERNSYM and an
     *        index, coding for where the right hand side of a rule begins.
     * @param v vector of tokens
     * @param sep rule side seperator (e.g. "-->")
     * @return std::pair of a vector of vectors of elements of type
     *         \b EXTERNSYM and an index
     */
    std::pair<std::vector<ESVec>, int> operator()(ESVec& v, ES sep)
    {
        std::vector<ESVec> sides;
        ESVec current;
        for (auto es = v.begin(); es != v.end(); ++es)
        {
            if (*es != sep)
            {
                current.push_back(*es);
            }
            else
            {
                current.shrink_to_fit();
                sides.push_back(current);
                current.clear();
            }
        }
        current.shrink_to_fit();
        sides.push_back(current);
        return std::make_pair(sides, 1);
    }
////////////////////////////////////////////////////////////////////////////////
}; // CFGRuleParser

/// hashes objects of type \b Rule<GRAMMAR>
template<typename GRAMMAR>
size_t hash_code(const Earley::Rule<GRAMMAR>& r)
{
    size_t hash = 0x9e3779b9;

    for (auto s = r.get_sides().begin(); s != r.get_sides().end(); ++s)
    {
        for (auto t = s->begin(); t!= s->end(); ++t)
        {
            hash += (((*t + (hash << 6)) ^ (hash >> 16)) - hash);
        }
    }

    return hash;
}

} // Earley

#endif //__RULE__HPP
