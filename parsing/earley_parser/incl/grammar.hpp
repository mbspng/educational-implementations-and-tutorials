/**
 * @file grammar.hpp
 * Generic grammar class. Which grammar type it conforms to is determined
 * by two external functors -- VALIDATOR and RULEPARSER. These determine
 * whether input strings of rule representations have the correct format
 * for the desired kind of grammar and parse these representations into
 * actual rule objects, which are a templated type that in turn depends
 * on this grammar class. The two functors allow the user to define the
 * grammar type according to his needs.
 * Rules are stored in a map that takes the left hand side of rules as
 * the key and then stores the whole rule in an unordered set as the
 * value.
 * All symbols (namely syntactic category labels) are translated into
 * an internal representation, which is defined in RULEPARSER.
 * The purpose is to allow faster lookup times, hence an integer type
 * should be chosen.
 * Input symbols need to be of another type defined in RULEPARSER -- the
 * type of external symbols.
 *
 * Matthias Bisping
 *
 * compilers: - clang-700.0.72
 *            - clang 3.5.2 / 3.5.0
 *            - GCC 5.2.0 / 4.8.3
 *            - Microsoft C/C++ 19.00.23026 for x86
 */

#ifndef __GRAMMAR__HPP
#define __GRAMMAR__HPP

#ifndef NDEBUG
#define NDEBUG
#endif

#include "declarations.hpp"

#include <map>
#include <set>
#include <unordered_set>
#include <fstream>

#include "helper.hpp"
#include "rule.hpp"
#include "load.hpp"
#include "translator.hpp"

// forward declarations
namespace Earley
{
    template<typename GRAMMAR>
    class Rule;

    template <typename VALIDATOR, typename RULEPARSER>
    class Grammar;

    template <typename GRAMMAR>
    std::size_t hash_code(const Earley::Rule<GRAMMAR>&);
}

// hasher for \b Earley::Rule<GRAMMAR>
namespace std
{
    /// hash template definition for objects of type \b Earley::Rule<GRAMMAR>
    template <typename GRAMMAR>
    struct hash<Earley::Rule<GRAMMAR>>
    {
        size_t operator()(const Earley::Rule<GRAMMAR>& r) const
        {
            return hash_code(r);
        }
    };
}

namespace Earley
{
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                     Grammar                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  class for representing a phrase struture grammar
 * @tparam VALIDATOR the type of the rule validator, checks rule
 *         representations for validity
 * @pre    @p VALIDATOR requires bool operator(std::vector<ES>& rule,
 *         ES separator)
 * @tparam RULEPARSER the type of the rule parser; processes rule
 *         representations
 * @pre    @p RULEPARSER requires std::pair<std::vector<ESVec>, int>
 *         operator()(ESVec& v, ES separator)
 */
template <typename VALIDATOR, typename RULEPARSER>
class Grammar
{
////////////////////////////////////////////////////////////////////////////////
public:                                                    //    PUBLIC TYPEDEFS
////////////////////////////////////////////////////////////////////////////////
typedef RULEPARSER                                                         RPar;
typedef VALIDATOR                                                           Val;
/// the type defined in the \b RuleParser for internal symbols
typedef typename RPar::IS                                                    IS;
/// the type defined in the \b RuleParser for external symbols
typedef typename RPar::ES                                                    ES;
/// vector of internal symbols
typedef typename RPar::ISVec                                              ISVec;
/// vector of external symbols
typedef typename RPar::ESVec                                              ESVec;
/// the \b Rule type for this \b Grammar
typedef typename Earley::Rule<Earley::Grammar<Val, RPar>>                  Rule;
#if !(SOVERLOAD)
/// type of Translator from IS to ES and vice versa
typedef Translator<IS, ES>                                                Trans;
#else
/// translation of ES to IS
typedef std::map<ES, IS>                                                ESISMap;
/// translation from IS to ES
typedef std::map<IS, ES>                                                ISESMap;
#endif
/// set of \b IS
typedef std::set<IS>                                                     ISSET;
/// set of rules -- value for \b RRMap
typedef std::unordered_set<Rule>                                        Ruleset;
/// type of the sides of \b Rule
typedef typename Rule::Ruleside                                        Ruleside;
/// type of the sides-field of \b Rule
typedef typename Rule::RulesideVec                                  RulesideVec;
/// maps left hand side of \b Rule to set of \b Rule
/// all rules with the same left hand side are stored under the same key
// eine Hashmap wäre vermutlich deutlich schneller, aber std::hash fuer
// Ruleside habe ich nicht zum Lompilieren bekommen
typedef std::map<Ruleside, Ruleset>                                       RRMap;
////////////////////////////////////////////////////////////////////////////////
public:                                                    //     PUBLIC METHODS
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief constructs empty grammar, containing but 1 rule - the start rule
     * @param ss super start symbol type \b ES
     * @param s start symbol of type \b ES
     * @param separator rule sides separating symbol of type \b ES
     */
    Grammar(ES ss="$", ES s="S", ES separator="-->")
    :
    del(helper::init("token_delimeter")),
    separator(separator),
    start(make_rule(ss+del+separator+del+s))
    {
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief constructs grammar from input stream of rules
     * @pre   one rule must occupy exactly one line
     * @param is ifstream with grammar rules
     * @param separator rule sides separating string
     * @param ss super start symbol
     * @param s start symbol
     */
    Grammar(std::ifstream& is, ES ss="$", ES s="S", ES separator="-->")
    :
    // add ss and s and separator to esism
    del(helper::init("token_delimeter")),
    separator(separator),
    start(make_rule(ss+del+separator+del+s))
    {
        fill(is);
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief injects a lexicon into the \b Grammar
     * @param lexicon the lexicon
     */
    void inject_lexicon(ISSET& lexicon)
    {
        this->lexicon = lexicon;
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief makes rule from a representation in std::string
     * @param repr rule representation
     */
    Rule make_rule(sstr repr)
    {
        ESVec tokens = helper::tokenise(repr);
        if (validator(tokens, separator))
        {
            // make a pair of rulesides and rhs_begin index
            std::pair<RulesideVec, int> rp = parse(tokens);
            return Rule(this, rp.first, rp.second);
        }else { malform_error(tokens); }
        exit(1);
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief parses a vector of type \b ES into a std::pair of \b RulesideVec
     *        and \b rhs_begin
     * @param toks vector of tokens of type \b ES
     * @return std::pair of \b RulesideVec and an index
     */
    std::pair<RulesideVec, int> parse(ESVec& toks)
    {
        // get a pair of rule sides and an index for where the RHS begins
        std::pair<std::vector<ESVec>, int> rp = ruleparser(toks, separator);
        // now translate ES to IS
        std::vector<ISVec> sides;
        ISVec current;
        for (auto esv = rp.first.begin(); esv != rp.first.end(); ++esv)
        {
            for (auto es = esv->begin(); es != esv->end(); ++es)
            {
                IS is = translate(*es);
                current.push_back(is);
            }
            current.shrink_to_fit();
            sides.push_back(current);
            current.clear();
        }
        // bind the translated sides-vector and the index in a pair
        return std::make_pair(sides, rp.second);
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief fills the grammar from a file of rules
     * @pre   one rule must occupy exactly one line
     * @param is ifstream with grammar rules
     */
    void fill(std::ifstream& is)
    {
        LOAD::Progressbar l(get_rulecount(is));
        int i = 0;
        std::string repr;
        // go over file and make \b Rule objects from string representations
        while(std::getline(is, repr))
        {
            // update progress bar
            l.run(i++);
            // skip empty lines
            if (repr.size() == 0) continue;
            // make a rule from string representation and add it to grammar
            insert(make_rule(repr));
        }
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief returns the set of rules that have \p lhs as their left hand side
     * @param lhs left hand side to look up all rules sharing it
     */
    const Ruleset& operator[](ISVec& lhs) const
    {
        static Ruleset empty_ruleset;
        auto r = rules.find(lhs);
        if (r == rules.end()) return empty_ruleset;
        return r->second;
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief sends string representation of grammar @p g to stream @p o
     * @param o stream so send to
     * @param g grammar so send representation of
     */
    friend sost& operator<<(sost& o, Grammar& g)
    {
        for(auto i = g.rules.begin(); i != g.rules.end(); ++i)
        {
            for(auto j = i->second.begin(); j != i->second.end(); ++j)
            {
                o << *j << "\n";
            }
        }
        return o;
    }
////////////////////////////////////////////////////////////////////////////////
    /// @returns separator
    sstr get_separator()
    {
        return separator;
    }
////////////////////////////////////////////////////////////////////////////////
    /// @returns \b IS translation of \b separator
    IS get_separatorID()
    {
        return translate(separator);
    }
////////////////////////////////////////////////////////////////////////////////
#if !(SOVERLOAD)
    /**
     * @brief translates instance @p is of type \b IS into an instance of type
     *        \b ES
     *        Throws an error if @p is is unkown
     * @param is token to translate
     * @return \b ES translation of @p is
     */
    ES translate(const IS& is) { return translator.translate(is); }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief translates instance @p es of type \b ES into an instance of type
     *        \b IS
     *        Adds an entry for @p es into \b esism if @p es is not known yet
     * @param es token to translate
     * @return \b IS translation of @p es
     */
    IS translate(const ES& es)
    {
        return translator.translate(es);
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief translates instances of type \b ES into instances of
     *        type \b IS
     * @param esv vector of ES tokens to translate
     * @return \b ISVec of translated tokens
     */
    ISVec translate(ESVec esv)
    {
        ISVec isv;
        for (auto t = esv.begin(); t != esv.end(); ++t)
        {
            isv.push_back(translator.translate(*t));
        }
        isv.shrink_to_fit();
        return isv;
    }
////////////////////////////////////////////////////////////////////////////////
#else
    /**
     * @brief translates instance @p es of type \b ES into an instance of type
     *        \b IS
     *        Adds an entry for @p es into \b esism if @p es is not known yet
     * @param es token to translate
     * @return \b IS translation of @p es
     */
    IS translate(const ES& es)
    {
        auto s = esism.find(es);
        if (s == esism.end())
        {
            IS is = esism.size()+1;
            esism.insert(std::make_pair(es, is));
            isesm.insert(std::make_pair(is, es));
            return is;
        }
        return s->second;
    }
////////////////////////////////////////////////////////////////////////////////
    ES translate(const IS& is) const
    {
        auto s = isesm.find(is);
        if (s == isesm.end())
        {
           static ES empty_ES = "<$>";
           return empty_ES;
        }
        return s->second;
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief translates instances of type \b ES into instances of
     *        type \b IS
     * @param esv vector of ES tokens to translate
     * @return \b ISVec of translated tokens
     */
    ISVec translate(ESVec esv)
    {
        ISVec isv;
        for (auto t = esv.begin(); t != esv.end(); ++t)
        {
            isv.push_back(translate(*t));
        }
        isv.shrink_to_fit();
        return isv;
    }
#endif
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief @returns wheter is is a known word of the grammar
     * @param is \b IS to look up in \b isesm
     */
    bool is_word(const IS& is)
    {
        return lexicon.find(is) != lexicon.end();
    }
////////////////////////////////////////////////////////////////////////////////
private:                                                   //    PRIVATE METHODS
////////////////////////////////////////////////////////////////////////////////
    /// takes a token vector and prints an error message containing
    /// the vector's tokens
    void malform_error(const ESVec& v) const
    {
        helper::msg("error:","malformed grammar rule '"+
                     helper::to_string(v)+"'",
                    __FILE__,__LINE__);
        exit(1);
    }
////////////////////////////////////////////////////////////////////////////////
    /// estimates number of rule representations in a stream object @p is
    /// (actually only counts lines)
    int get_rulecount(std::ifstream& is)
    {
        int rulecount = 0;
        std::string dummy;
        while(std::getline(is, dummy))
        {
            ++rulecount;
        }
        // make stream reiterable
        is.clear();
        is.seekg(0, is.beg);
        return rulecount;
    }
////////////////////////////////////////////////////////////////////////////////
    /// inserts rule into rules
    void insert(const Rule& r)
    {
        rules[*(r.get_lhs())].insert(r);
    }
////////////////////////////////////////////////////////////////////////////////
public:                                                    //      PUBLIC FIELDS
////////////////////////////////////////////////////////////////////////////////
    const sstr del;     ///< rule token delimiter
    #if !(SOVERLOAD)
    /// translates internal to external symbols and vice versa
    Trans translator;
    #else
    ESISMap esism;      ///< maps external symbols to internal ones
    ISESMap isesm;      ///< maps internal symbols to external ones
    #endif
    const ES separator; ///< rule sides separating symbol
    ISSET lexicon;        ///< storage of all words in the lexicon
    const Rule start;   ///< start rule
////////////////////////////////////////////////////////////////////////////////
private:                                                   //     PRIVATE FIELDS
////////////////////////////////////////////////////////////////////////////////
    /// maps left hand side of \b Rule to set of \b Rule
    /// all rules with the same left hand side are stored under the same key
    RRMap rules;
    /// validates rule representations
    Val validator;
    /// parses rule representations
    RPar ruleparser;
////////////////////////////////////////////////////////////////////////////////
}; // CFG
/**
 * @brief tests whether a string can be transformed into a valid CFG rule
 * @pre requires a std::vector<std::string> of tokens as input
 */
template <typename ES>
struct CFGValidator
{
    bool operator()(std::vector<ES>& v, ES separator) const
    {
        // if the first token that would become the LHS is not empty,
        // the second token is the rule side separator specified for
        // the grammar and there are more than 2 tokens total and not
        // more than 1 separator then the string can become a valid CFG
        // rule.
        if(v.size() > 2)
        {
            if (v[0].size() != 0 && v[1] == separator)
            {
                int sep = 0;
                int token = 0;
                for(auto i = v.begin(); i != v.end(); ++i, ++token)
                {
                    if (*i == separator) ++sep;
                    // if more than 1 token has been read and no
                    // separator yet, then the left hand side of the
                    // rule would have 2 tokens
                    if (sep == 0 && token == 1) return false;
                }
                if (sep == 1) return true;
            }
        }
        return false;
    }
}; // CFGValidator

} // Earley

#endif //__GRAMMAR__HPP
