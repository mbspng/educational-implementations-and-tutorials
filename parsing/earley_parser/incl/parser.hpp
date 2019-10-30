/**
 * @file parser.hpp
 * implementation of the Earley Parser
 *
 * Matthias Bisping
 *
 * compilers: - clang-700.0.72
 *            - clang 3.5.2 / 3.5.0
 *            - GCC 5.2.0 / 4.8.3
 *            - Microsoft C/C++ 19.00.23026 for x86
 */

#ifndef __PARSER__HPP
#define __PARSER__HPP

#ifndef NDEBUG
#define NDEBUG
#endif

#include <assert.h>
#include <vector>
#include <set>
#include <fstream>
#include <unordered_set>
#include <unordered_map>

#include "declarations.hpp"
#include "helper.hpp"
#include "item.hpp"
#include "chart.hpp"
#include "busy.hpp"
#include "grammar.hpp"


namespace Earley
{
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                 EarleyParser                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief implementation of the Earley Parser
 * @tparam GRAMMAR grammar type to parse on
 * @pre GRAMMAR is required to be templated with Earley::CFGRuleParser<IS, ES>
 */
template <typename GRAMMAR>
class EarleyParser
{
////////////////////////////////////////////////////////////////////////////////
public:                                                    //    PUBLIC TYPEDEFS
////////////////////////////////////////////////////////////////////////////////
typedef GRAMMAR                                                         Grammar;
////////////////////////////////////////////////////////////////////////////////
private:                                                   //    PRIATE TYPEDEFS
////////////////////////////////////////////////////////////////////////////////
/// the Rule type for this \b Parser
typedef typename Grammar::Rule                          Rule;
/// the Chart type for this \b Parser
typedef EarleyChart<EarleyParser>                       Chart;
/// the Item type for \b chart
typedef typename Chart::Item                            Item;
typedef std::vector<Rule>                               RuleVec;
typedef typename Rule::IS                               IS;
typedef typename Rule::ISVec                            ISVec;
typedef typename Rule::ESVec                            ESVec;
typedef typename Rule::ES                               ES;
typedef typename Grammar::Ruleset                       Ruleset;
typedef typename Chart::ItemSet                         ItemSet;
typedef typename std::set<IS>                           ISSet;
typedef typename std::map<IS, std::unordered_set<ES>>   TagID_Words_Map;
typedef typename Rule::RulesideVec                      RulesideVec;
////////////////////////////////////////////////////////////////////////////////
public:                                                    //     PUBLIC METHODS
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief constructs parser with grammar \p g, a set of POS-tags
     *        \p tags and a map from tags to word \p pwm
     */
    EarleyParser(Grammar& g, ISSet tags, TagID_Words_Map pwm)
    :grammar(g),
    tags(tags),
    pwm(pwm)
    {
        grammar_ptr = &grammar;
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief takes vector of ES tokens and parses it
     */
    bool parse(ESVec sentence)
    {
        // clear the chart (might be filled from previous sentence)
        chart.clear();
        // initialize the chart with the input and the start rule
        // of the grammar
        chart.initialise(sentence, grammar.start);

        bool new_p = false; // stores whether new items were predicted
        bool new_c = false; // stores whether new items were completed

        // loop over the chart
        for (short index = 0; index < chart.size(); ++index)
        {
            // initialize to_process with the items in the current cell
            // (start item for first cell and all scanned items for other cells)
            to_process.insert(chart[index].begin(), chart[index].end());
            // keep predicting, scanning, completing, as long as
            // new items can be added to the current cell
            do
            {
                new_p = false;
                new_c = false;
                for (auto item = to_process.begin(); item != to_process.end(); ++item)
                {
                    // update the busy indicator
                    bar.run();

                    /*
                     * A grammar might contain both rules 'A --> A' and 'A --> a'
                     * In that case, it is necessary to apply predict() to the
                     * former rule, because the 'A' of the RHS doubles as a
                     * POS-tag and as a complex syntactic category.
                     * Though 'A --> a' ought not be among the predicted rules,
                     * as to not flood the cells with terminal rules like it.
                     * For this case it is necessary to predict for rules that
                     * have symbols that are in the tagset of the grammar as their
                     * LHS, but to not add terminal rules to the cell.
                     * The test for that is inside predict(), the test immediately
                     * below just ensures these kinds of rules get passed to
                     * predict() in the first place.
                     */

                     #if SOVERLOAD
                    // in case SOVERLOAD is enabled, the parser will not check
                    // whether item.next() is a tag
                    if (!item->complete())
                    #else

                    /*
                     * If a grammar does not contain both rules 'A --> A' and
                     * 'A --> a', then rules with a POS-tag at the dot index do
                     * not need to be passed to the predict() function, as there
                     * are no rules in the grammar to predict from a POS-tag
                     * anyway.
                     * If the grammar DOES contain terminal rules, it needs to
                     * be avoided to predict anything for said rules that have
                     * a POS-tag at the dot index, as there are indeed rules to
                     * predict, namely all the terminal rules that have the
                     * symbol at dot index as their LHS. But it is not desired
                     * to predict terminal rules. Therefore these need to be
                     * filtered out.
                     */

                    // if SOVERLOAD is not enabled, the parser will filter out
                    // all rules that have the dot at a POS-tag, before passing
                    // them to predict()
                    if (!item->complete() && tags.find(item->next()) == tags.end())
                    #endif

                    {
                        // predict items
                        if(predict(*item)) new_p = true;
                    }
                    // if the symbol at dot index is a POS-tag...
                    if (!item->complete() && tags.find(item->next()) != tags.end())
                    {
                        // add terminal rules to the next cell
                        // by scanning rules in this cell
                        scan(*item);
                    }
                    // if the item is complete (has the dot behind its last RHS
                    // symbol)...
                    else if (item->complete())
                    {
                        // complete rules in this cell
                        if(complete(*item)) new_c = true;
                    }
                }
                // merge the items that have been processed in this
                // iteration with the current cell
                merge(index);
            }
            while(new_p || new_c);
        }
        // clear the busy indicator
        bar.cancel();
        // determine, whether the string could be derived
        return ((chart.end()-1)->find(chart.get_final()) != (chart.end()-1)->end());
    }
////////////////////////////////////////////////////////////////////////////////
    /// sends representation of the chart to stream @p o
    void show_chart(sost& o=std::cout)
    {
        chart.show(o);
    }
////////////////////////////////////////////////////////////////////////////////
    /// @returns a copy of the \b chart
    Chart get_chart() const
    {
        return chart;
    }
////////////////////////////////////////////////////////////////////////////////
private:                                                   //    PRIVATE METHODS
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief   completes items
     * @details adds all items in the cell specified as @p items left span border
     *          that have their dot at the a symbol the syntactic category of
     *          which is the same as @p item's LHS will be copied into the current
     *          cell with the dot advanced by one position.
     * @param   item on the basis of which to potentially complete others
     */
    bool complete(const Item& item)
    {
        bool any_new = false; // stores whether any items were completed
        // ... look up all items in the cell the current item has
        // specified as its 'from' value, that have the dot at the
        // same symbol that is the LHS of the current item
        for (auto item2 = chart[item.from].begin();
             item2 != chart[item.from].end();
             ++item2)
        {
            // check if LHS of current item is symbol at dot index
            // of item2
           if (!item2->complete() && item.get_lhs() == item2->next())
           {
                // if so, make a completed icon from it
                Item item3(item2->rule, item2->dot+1, item2->from, item.to);
                assert(item.to == item.to && "left span border != item.to");
                // if this item is not present yet, add it to complete_buffer
                if(!chart.contains(item.to, item3) &&
                   to_process.find(item3) == to_process.end() &&
                   complete_buffer.find(item3)==complete_buffer.end())
                {
                    complete_buffer.insert(item3);
                    any_new = true;
                }
            }
        }
        return any_new;
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief   scans items
     * @details inspects the argument item @p item. If it has the dot at a symbol
     *          the syntactic category of which is a POS-tag, then a new item
     *          will be added to the next chart cell, if furthermore the syntactic
     *          category is a possible category of the word in the current cell.
     * @param   item on the basis of which to potentially add a new one
     *          to the next chart cell
     */
    void scan(const Item& item)
    {
        // test whether the token that corresponds with the current cell
        // is in the words of the POS-tag at dot index of item
        auto  pw_it = pwm.find(item.next());
        if (pw_it != pwm.end() &&
           pw_it->second.find(chart.get_word(item.to)) != pw_it->second.end())
        {
            // translate the word of the current cell into an IS
            IS wordID = grammar.translate(chart.get_word(item.to));
            // make a rule from the pos tag and the word at index
            RulesideVec rsv = {ISVec(1, item.next()), ISVec(1, wordID)};
            // call the constructor of Rule, that builds a rule form
            // a vector of sides
            // make an item with 'to' set to the index of the word+1
            Item item2(Rule(grammar_ptr, rsv), 1, item.to, item.to+1);
            assert (item.to+1 <= chart.size() && "chart ubervoll");
            // add the new item to the next chart cell
            chart[item.to+1].insert(item2);
        }
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief   predicts items
     * @details adds predicted items to predict_buffer
     * @param   item the item on the basis of which to potentially predict
     *          new ones
     */
    bool predict(const Item& item)
    {
        bool any_new = false; // stores whether any items were predicted

        // lookup all rules that have item.next() as their LHS
        ISVec s = {item.next()};
        Ruleset rs = grammar[s];
        // iterate over all rules in the Ruleset
        for (auto r = rs.begin(); r != rs.end(); ++r)
        {
            /*
             * If SOVERLOAD is enabled, all rules that are terminal rules will
             * be filtered out. If SOVERLOAD is not enabled, the parser assumes
             * there to BE no terminal rules in the grammar. If however there are
             * indeed  terminal rules in the grammar and SOVERLOAD is not enabled,
             * all cells will be flooded with terminal rules of the form 'A --> a',
             * 'a' being a word, rather than a category symbol.
             * All rules that have as their LHS a symbol taht doubles as a POS-tag
             * and a complex syntactic category ('A --> A', 'A --> a') will be let
             * through ( as well as all other normal rules like 'B --> C').
             */
             #if SOVERLOAD
             if (grammar.is_word(*(r->get_rhs()->begin()))) continue;
             #endif
/*
**********************************************************************
* this would be necessary if the map in the grammar, that maps LHS to
* sets of rules was an unordered one:
*
*              // as several LHS might have been mapped to the same
*              // hash, test whether the r's LHS is the  same as s
*              if (s == *(r->get_lhs()))
*              {
**********************************************************************
*/
                // make an item from every rule and add it to the
                // current chart cell, if it is not present for this
                // cell yet
                Item item2(*r, 0, item.to, item.to);
                // if this item is not present yet, add it to predict_buffer
                if(!chart.contains(item.to, item2) &&
                   to_process.find(item2) == to_process.end() &&
                   predict_buffer.find(item2) == predict_buffer.end())
                {
                    predict_buffer.insert(item2);
                    any_new = true;
                }
//          }
        }
    return any_new;
  }
////////////////////////////////////////////////////////////////////////////////
    void merge(short index)
    {
        chart[index].insert(to_process.begin(), to_process.end());
        to_process.clear();
        to_process.insert(predict_buffer.begin(), predict_buffer.end());
        to_process.insert(complete_buffer.begin(), complete_buffer.end());
        complete_buffer.clear();
        predict_buffer.clear();
    }
////////////////////////////////////////////////////////////////////////////////
private:                                                   //     PRIVATE FIELDS
////////////////////////////////////////////////////////////////////////////////
    /// grammar to parse with
    Grammar grammar;
    /// pointer grammar
    Grammar* grammar_ptr = nullptr;
    /// chart of \b Earley::EarleyItem<RULE>
    Chart chart;
    /// set of POS-tags from \p grammar
    const ISSet tags;
    /// maps tags to words
    const TagID_Words_Map pwm;
    /// sign of life in case of long derivation
    BUSY::Variant2 bar;
    /// buffers new items, so iterators don't get invalidated
    ItemSet predict_buffer;
    /// buffers new items, so iterators don't get invalidated
    ItemSet complete_buffer;
    /// buffers new items, so iterators don't get invalidated
    ItemSet scanned_buffer;
    /// buffers all newly made items
    ItemSet to_process;

}; // EarleyParser

} // Earley

#endif // __PARSER__HPP
