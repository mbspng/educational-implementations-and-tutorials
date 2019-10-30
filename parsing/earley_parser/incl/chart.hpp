/**
 * @file chart.hpp
 * Chart class for earley parser. Wraps a vector of sets of
 * \b Earley::EarleyItem<RULE> as a parse cahrt.
 *
 * Matthias Bisping
 *
 * compilers: - clang-700.0.72
 *            - clang 3.5.2 / 3.5.0
 *            - GCC 5.2.0 / 4.8.3
 *            - Microsoft C/C++ 19.00.23026 for x86
 */

#ifndef __CHART__HPP
#define __CHART__HPP

#ifndef NDEBUG
#define NDEBUG
#endif

#include <assert.h>
#include <vector>
#include <unordered_set>
#include <fstream>

#include "declarations.hpp"
#include "helper.hpp"
#include "item.hpp"

namespace Earley
{
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                 EarleyChart                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief \b Chart for the \b Earley::Parser<GRAMMAR> as a vector of sets
 *        of \b Items. The \b Chart has a specific start \b Item, that
 *        depends on input from the \b Parser as well as a derived final
 *        \b Item.
 *        If that \b Item is placed into the last \b Chart cell during the
 *        parse, the input string for the parser will have been parsed
 *        successfully.
 */
template <typename PARSER>
class EarleyChart
{
////////////////////////////////////////////////////////////////////////////////
public:                                                     //   PUBLIC TYPEDEFS
////////////////////////////////////////////////////////////////////////////////
/// the \b Rule type for this \b Chart
typedef typename PARSER::Grammar::Rule                                     Rule;
/// the \b Item type for this \b Chart
typedef EarleyItem<Rule>                                                   Item;
/// set of \b Items for the cells of \b chart
typedef std::unordered_set<Item>                                        ItemSet;
/// The type for the internal \b chart
typedef std::vector<ItemSet>                                              Chart;
/// the type defined in the \b Rule for internal symbols
typedef typename Rule::IS                                                    IS;
/// the type defined in the \b Rule for external symbols
typedef typename Rule::ES                                                    ES;
/// vector of internal symbols
typedef typename Rule::ISVec                                              ISVec;
/// vector of external symbols
typedef typename Rule::ESVec                                              ESVec;
////////////////////////////////////////////////////////////////////////////////
public:                                                     //    PUBLIC METHODS
////////////////////////////////////////////////////////////////////////////////
    /// constructs empty chart
    EarleyChart()
    {
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief fills \b tokens from vector of \b ES tokens @p sentence and
     *        inserts start \b Item into \b Chart. Also defines \b final_item
     *        as a completed of the start \b Item.
     * @param sentence vector of tokens to parse
     * @param startrule the \b startrule defined for the \b grammar
     */
    void initialise(ESVec& sentence, Rule startrule)
    {
        // fill \b tokens with tokens from @p sentence
        tokens = sentence;
        chart.resize(tokens.size()+1);
        tokens.push_back("$");
        insert(0, Item(startrule));
        final_item = Item(startrule, startrule.get_rhs()->size(), 0, chart.size()-1);
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief fills \p tokens from stream of tokens \p is, inserts inserts
     *        start \b Item into \b Chart. Also defines \b final_item
     *        as a completed of the start \b Item.
     * @param is stream of tokens to parse
     * @param startrule the startrule defined for the grammar
     */
    void initialise(std::stringstream& is, Rule startrule)
    {
        // fill \p tokens with tokens from stream
        std::string token;
        while(std::getline(is, token))
        {
            // skip empty lines
            if (token.size() == 0) continue;
            tokens.push_back(token);
        }
        chart.resize(tokens.size()+1);
        // -1 is an impossible IS, it serves as an EOS marker
        tokens.push_back(-1);
        // make and \b Item from the \b start rule and add it to the first
        // cell of the \b chart
        insert(0, Item(startrule));
        // define the final \b Item as a completed version of the start \b Item
        final_item = Item(startrule, startrule.get_rhs()->size(), 0, chart.size()-1);
    }
////////////////////////////////////////////////////////////////////////////////
    /// resets chart and tokens
    void clear()
    {
        chart.clear();
        tokens.clear();
    }
////////////////////////////////////////////////////////////////////////////////
    /// @returns \b final_item
    const Item& get_final() const
    {
        return final_item;
    }
////////////////////////////////////////////////////////////////////////////////
    /// @returns first cell of \b chart as an iterator
    const typename Chart::iterator begin()
    {
        return chart.begin();
    }
////////////////////////////////////////////////////////////////////////////////
    /// @returns last cell of \b chart as an iterator
    const typename Chart::iterator end()
    {
        return chart.end();
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief inserts \b Item \p item into \b chart at index \p index
     * @param item \b Item to insert
     * @param index cell index of \b chart to insert @p item into
     */
    void insert(short index, Item item)
    {
        // if the chart has fewer cells than the index, resize it
        int length = chart.end() - chart.begin();
        if (index >= length)
        {
            ItemSet is;
            is.insert(item);
            chart.resize(index);
            chart[index].insert(item);
        }
        else
        {
            chart[index].insert(item);
        }
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief  tests whether \b chart contains \p item at index
     *         \p index
     * @param  item \b Item to check presence of for
     * @param  index cell index of \b chart to look in for \p item
     * @return true if \p item was found in \b chart at \p index, false
     *         else
     */
    bool contains(short unsigned index, Item item)
    {
        if (index >= chart.size())
        {
            helper::msg("error:",
                        "index ["+helper::to_string(index)+"] exceeds array",
                        __FILE__, __LINE__);
            exit(1);
        }
        else
        {
            return (chart[index].find(item) != chart[index].end());
        }

    }
////////////////////////////////////////////////////////////////////////////////
    /// @return lengths of \p chart
    short size()
    {
        assert(tokens.size() == chart.size() && "tokens+1 != chart");
        return chart.size();
    }
////////////////////////////////////////////////////////////////////////////////
    /// @return set of items of \b chart at @p index
    ItemSet& operator[](short unsigned index)
    {
        if (index >= chart.size())
        {
            helper::msg("error:",
                        "index ["+helper::to_string(index)+"] exceeds array",
                        __FILE__, __LINE__);
            exit(1);
        }
        return chart[index];
    }
////////////////////////////////////////////////////////////////////////////////
    /// @return word in \p tokens at index \p index
    ES get_word(short unsigned index)
    {
        if (index >= tokens.size())
        {
            helper::msg("error:",
                        "index ["+helper::to_string(index)+"] exceeds array",
                        __FILE__, __LINE__);
            exit(1);
        }
        return tokens[index];
    }
////////////////////////////////////////////////////////////////////////////////
    /// sends a representation of the chart to \p o
    void show(sost& o=std::cout)
    {
        assert(tokens.size() == chart.size() && "tokens != chart");
        int i = 0;
        o << "\n";
        // loop over the \b chart cells
        for (auto cell = chart.begin(); cell !=chart.end(); ++cell, ++i)
        {
            // loop over the \b Items in the cell
            o << "CHART[" << i << "] ('" << tokens[i] << "')\n\n";
            for (auto item = cell->begin(); item != cell->end(); ++item)
            {
                o << *item << "\n";
                o.flush();
            }
            helper::fill_line('_');
            o << "\n";
        }
        o << "\n";
    }
////////////////////////////////////////////////////////////////////////////////
private:                                                    //    PRIVATE FIELDS
////////////////////////////////////////////////////////////////////////////////
    Chart chart;     ///< the parse chart containing items
    ESVec tokens;    ///< the tokens from in input phrase
    Item final_item; ///< the completed start item
////////////////////////////////////////////////////////////////////////////////
}; // EarleyChart

} // Earley

#endif // __CHART__HPP
