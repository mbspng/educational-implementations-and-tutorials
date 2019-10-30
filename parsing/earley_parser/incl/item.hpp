/**
 * @file item.hpp
 * Earley item class. Wraps a \b Earley::Rule<GRAMMAR> object. Wrapped
 * \b Rules appear as if they were dotted rules, as \b Item has a dot
 * index field \b dot. \b Rules are assumed to be constructed according
 * to \b Earley::CFGRuleParser<IS, ES>.
 *
 * Matthias Bisping
 *
 * compilers: - clang-700.0.72
 *            - clang 3.5.2 / 3.5.0
 *            - GCC 5.2.0 / 4.8.3
 *            - Microsoft C/C++ 19.00.23026 for x86
 */

#ifndef NDEBUG
#define NDEBUG
#endif

#ifndef __ITEM__HPP
#define __ITEM__HPP

#include <iostream>
#include <vector>
#include <ostream>

#include "helper.hpp"
#include "declarations.hpp"
#include "rule.hpp"

namespace Earley
{
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                  EarleyItem                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Earley item class. Wraps a \b Earley::Rule<GRAMMAR> object.
 *        Wrapped \b Rules appear as if they were dotted rules, as \b
 *        Item has a dot index field \b dot. \b Rules are assumed to be
 *        constructed according to \b Earley::CFGRuleParser<IS, ES>.
 * @tparam RULE the \b Earley::Rule<GRAMMAR> type for this \b Item
 */
template <typename RULE>
class EarleyItem
{
////////////////////////////////////////////////////////////////////////////////
public:                                                      //     PUBLIC TYPES
////////////////////////////////////////////////////////////////////////////////
/// the \b Rule type of this \b Item
typedef RULE                                                               Rule;
/// the internal symbol type used in \b Rule
typedef typename Rule::IS                                                    IS;
////////////////////////////////////////////////////////////////////////////////
public:                                                      //   PUBLIC METHODS
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief constructs empty \b Item
     */
    EarleyItem()
    :
    dot(0),
    from(0),
    to(0)
    {
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief constructs \b Item from a \b Rule \p rule
     * @param rule the \b Rule from which to construct the \b Item
     * @param dot the dot position of the \b Item
     * @param from the left border of the span the \b  Item covers
     * @param to the right border of the span the \b  Item covers
     */
    EarleyItem(const Rule& rule, short dot=0, short from=0, short to=0)
    :
    dot(dot),
    from(from),
    to(to),
    rule(rule)
    {
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief sends representation of \b Item @p item to stream @p o
     * @param o stream to send to
     * @param item \b Item to send representation of to @p o
     */
    friend sost& operator<<(sost& o, const EarleyItem& item)
    {
        // translate and send left hand side
        o << item.rule.translate_to_es(*(item.rule.get_lhs()->begin())) << " ";
        // send separator symbol
        #ifdef UNIXLIKE
        o << "\t⟶\t";
        #else
        o << "\t-->\t";
        #endif
        // translate and send right hand side up to the dot
        auto j = (item.rule.get_rhs())->begin();
        for (int i = 0; i < item.dot; ++i)
        {
            o << item.rule.translate_to_es(*j) << " ";
            ++j;
        }
        // send the dot
        #ifdef UNIXLIKE
        o << "•";
        #else
        o << ".";
        #endif
        // translate and send right hand side from the dot on
        for (j = (item.rule.get_rhs())->begin()+item.dot;
             j != (item.rule.get_rhs())->end();
             ++j)
        {
            o << " " << item.rule.translate_to_es(*j);
        }
        return o;
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief compares 2 \b Items. \b Items are identical if their rule,
     *        dot index and their left and right span border are identical
     * @param i \b Item to compare to \b *this
     */
    bool operator==(const EarleyItem& i) const
    {
        return (i.rule == rule &&
                i.dot  == dot  &&
                i.from == from &&
                i.to   == to);
    }
////////////////////////////////////////////////////////////////////////////////
    /// @returns true, if \b Item is complete
    bool complete() const
    {
        return dot >= (int)rule.get_rhs()->size();
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @return the symbol of the RHS of \p rule at the dot position
     */
    IS next() const
    {
        return *(rule.get_rhs()->begin()+dot);
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief accesses \b rule's right hand side at position @p index.
     *        Indexing is defined only for \b Rules' RHSs
     * @pre   assumes \b rule to be organized according to the preset
     *        rule parser \b Earley::CFGRuleParser<IS, ES>
     */
    const IS& operator[](short index) const
    {
        if (index >= rule.get_rhs().size())
        {
            helper::msg("error:",
                        "index ["+helper::to_string(index)+"] exceeds array",
                        __FILE__, __LINE__);
            exit(1);
        }
        rule.get_rhs()[index];
    }
////////////////////////////////////////////////////////////////////////////////
    EarleyItem& operator=(const EarleyItem& src)
    {
        if (this != &src)
        {
            rule = src.rule;
            dot  = src.dot;
            from = src.from;
            to   = src.to;
        }
        return *this;
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief accesses \p rule's LHS
     * @pre   assumes \p rule to be organized according to the preset
     *        rule parser \p Earley::CFGRuleParser
     */
    const IS& get_lhs() const
    {
        return *((rule.get_lhs())->begin());
    }
////////////////////////////////////////////////////////////////////////////////
public:                                                      //    PUBLIC FIELDS
////////////////////////////////////////////////////////////////////////////////
    short dot;  ///< index of dot
    short from; ///< stores left span border
    short to;   ///< stores right span border
    Rule rule;  ///< \b Rule instance; main content of \b Item
////////////////////////////////////////////////////////////////////////////////
}; // EarleyItem

} // Earley

namespace std
{
using namespace helper;
/// hash template definition for objects of type \b Earley::EarleyItem<RULE>
template<typename RULE>
struct hash<Earley::EarleyItem<RULE>>
{
    size_t operator()(const Earley::EarleyItem<RULE>& i) const
    {
        return hash_combine(i.from+i.to, hash_combine(i.dot, i.rule));
    }
};

}

#endif //__ITEM__HPP
