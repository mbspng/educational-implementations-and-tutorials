/*
 * Driver program for parser class. Parser can be compiled to work with
 * terminal rules (e. g. 'V --> goes') or without, depending on what is
 * required given a certain grammar.
 *
 * Matthias Bisping
 *
 * compilers: - clang-700.0.72
 *            - clang 3.5.2 / 3.5.0
 *            - GCC 5.2.0 / 4.8.3
 *            - Microsoft C/C++ 19.00.23026 for x86
 */
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma warning(disable : 4503)
#endif

#include <iostream>
#ifdef UNIXLIKE
#include <unistd.h>
#endif
#include <stdio.h>
#include <set>
#include <unordered_set>
#include <map>
#include <fstream>

#include "../incl/parser.hpp"
#include "../incl/grammar.hpp"
#ifdef _WIN32
#include "../incl/getopt.h"
#include <io.h>
#endif


using namespace helper;
using namespace std;

// TODO: Redo command line argument interface with boost's program_options
// (https://www.boost.org/doc/libs/1_71_0/doc/html/program_options.html)

void usage()
{
    cerr << "Usage:\n"
    << "   ( -f <input file> | -s <input string> ) -g <grammar> -t <POS-tags> -w <words> [-v <verbosity>]\n"
    << "    -g <grammar> -t <POS-tags> -w <words> [-v <verbosity>] < <input stream>\n";
    exit(1);
}

void help()
{
    cerr << "\nEarley Parser\n\n"
    << "Usage:\n"
    << "    ( -f <input file> | -s <input string> ) -g <grammar> -t <POS-tags> -w <words> [-v <verbosity>]\n"
    << "    -g <grammar> -t <POS-tags> -w <words> [-v <verbosity>] < <input stream>\n"
    << "\nOptions:\n"
    << "    -f    file with text to parse; tokens separated by space or new line. Sentences separated by empty line\n"
    #if SOVERLOAD
    << "    -g    grammar (CFG) file; max 1 rule per line\n"

    #else
    << "    -g    grammar (CFG) file; max 1 rule per line. May NOT contain terminal rules for words (e.g. 'V --> goes')\n"
    #endif
    << "    -h    show this message\n"
    << "    -s    string to parse; tokens separated by spaces\n"
    << "    -t    POS-tag file; max 1 tag per line\n"
    << "    -v    verbosity [default: 0]\n"
    #if SOVERLOAD
    << "    -w    words file; max(min 1 token followed by exactly 1 tag) per line"
    #else
    << "    -w    words file; max(min 1 token followed by exactly 1 tag) per line. The terminal rules for words banned"
       " from the grammar are represented here.\n"
    #endif
    << "\n";
}

void input_error()
{
    helper::msg("error:","one input to parse only\n");
    exit(1);
}

void failed_to_open(sstr s)
{
    helper::msg("error","failed to open '"+ s+"'\n");
    exit(1);
}


int main(int argc, char* argv[])
{

    int verbosity = 0;

    ifstream grammarfile; // stream with grammar
    ifstream NTfile; // stream with all non-terminals
    ifstream tagfile; // stream with tags
    ifstream wordfile; // stream with words and tags
    ifstream inputstream; // input stream to parse
    string inputstring; // string with words to parse
    vector<string> sentence; // vector of tokens to parse
    vector<vector<string>> sentences; // vector with all sentences to parse

    int option;
    int iflag = 0;
    int gflag = 0;
    int tflag = 0;
    int wflag = 0;
    int vflag = 0;

    // show help if only -h is passed
    if (argc == 2)
    {
        while ((option = getopt(argc, argv, "h")) != -1)
            switch (option) {
                case 'h':
                    help();
                    exit(1);
                    break;

                default:
                    usage();
                    break;
            }
    }
    else if (argc >= 7 && argc < 12)
    {
        while ((option = getopt(argc, argv, "f:s:g:n:t:w:v:")) != -1)
        {
            switch (option) {
                case 'f':
                    if (!iflag)
                    {
                        inputstream.open(optarg, std::ifstream::in);
                        if (!inputstream.is_open()) failed_to_open(optarg);
                    }
                    else { input_error(); }
                    iflag++;
                    break;

                case 's':
                    if (!iflag) inputstring = optarg;
                    else { input_error(); }
                    iflag++;
                    break;

                case 'g':
                    if (!gflag)
                    {
                        grammarfile.open(optarg, std::ifstream::in);
                        if (!grammarfile.is_open()) failed_to_open(optarg);
                    }
                    else
                    {
                        helper::msg("error:","1 grammar file permissible at most\n");
                        exit(1);
                    }
                    gflag++;
                    break;

                case 't':
                    if (!tflag)
                    {
                        tagfile.open(optarg, std::ifstream::in);
                        if (!tagfile.is_open()) failed_to_open(optarg);
                    }
                    else
                    {
                        helper::msg("error:","1 tag file permissible at most\n");
                        exit(1);
                    }
                    tflag++;
                    break;

                case 'w':
                    if (!wflag)
                    {
                        wordfile.open(optarg, std::ifstream::in);
                        if (!wordfile.is_open()) failed_to_open(optarg);
                    }
                    else
                    {
                        helper::msg("error:","1 word file permissible at most\n");
                        exit(1);
                    }
                    wflag++;
                    break;

                case 'v':
                    if (!vflag) verbosity = atoi(optarg);
                    else
                    {
                        helper::msg("error:","verbosity level already specified");
                        exit(1);
                    }
                    vflag++;
                    break;

                default:
                    usage();
                    break;
            }
        }
        // tokenise input string
        if (inputstring.size() > 0)
        {
            sentence = helper::tokenise(inputstring);
            // append sentence to sentences
            sentences.push_back(sentence);
        }
        // tokenise input stream
        else if (inputstream.is_open())
        {
            string line;
            while(getline(inputstream, line))
            {
                svec_s line_tokens = helper::tokenise(line);
                // append line tokens to vector of tokens of single sentence
                sentence.insert(sentence.end(), line_tokens.begin(), line_tokens.end());
                if (line.size() == 0)
                {
                    // append sentence to sentences
                    sentences.push_back(sentence);
                    sentence.clear();
                }
            }
            sentences.push_back(sentence);
        }
        #ifdef _WIN32
        else if (!_isatty(_fileno(stdin)))
        #else
        else if (!isatty(STDIN_FILENO))
        #endif
        {
            string line;
            while(getline(cin, line))
            {
                cout << line << endl;
                svec_s line_tokens = helper::tokenise(line);
                // append line tokens to vector of tokens of single sentence
                sentence.insert(sentence.end(), line_tokens.begin(), line_tokens.end());
                if (line.size() == 0)
                {
                    // append sentence to sentences
                    sentences.push_back(sentence);
                    sentence.clear();
                }
            }
            sentences.push_back(sentence);
        }
        // arg count matches, but no input to parse
        else usage();
    }
    // arg count doesn't match
    else usage();


    typedef string                                 ES;
    typedef long                                   IS;
    typedef Earley::CFGRuleParser<IS, ES>          RP;
    typedef Earley::CFGValidator<RP::ES>           V;
    typedef Earley::Grammar<V, RP>                 GRAMMAR;
    typedef Earley::EarleyParser<GRAMMAR>          PARSER;

    // create grammar instance
    GRAMMAR g(grammarfile);

    set<IS> tag_set; // stores tags as IDs of type IS

    // build a set of tags
    string line;
    while(getline(tagfile, line))
    {
        if (line.size() == 0) continue;
        svec_s tokens = tokenise(line);
        if (tokens.size() != 1)
        {
            msg("error:", "'"+to_string(line)+"' in tags file. Invalid format",
                __FILE__, __LINE__);
            exit(1);
        }

        tag_set.insert(g.translate(line));
    }


    map<IS, unordered_set<ES>> TagID_Words_Map; // mapps tag IDs (which have type IS) to sets of words

    // if overloaded symbols are enabled, make a lexicon of all words
    #if SOVERLOAD
    set<IS> lexicon;
    #endif

    // build a map from tags to sets of words
    while(getline(wordfile, line))
    {
        if (line.size() == 0) continue;
        svec_s tokens = tokenise(line);
        if (tokens.size() < 2)
        {
            msg("error:", "'"+to_string(line)+"' in words file. Invalid format",
                __FILE__, __LINE__);
            exit(1);
        }
        sstr nl_string;
        for (auto i = tokens.begin(); i != tokens.end()-1; ++i)
        {
            nl_string += *i;
            if (!(i+1 == tokens.end()-1)) nl_string += " ";
        }
        // translate the tag into an ID of type IS
        IS tagID = g.translate(*(tokens.end()-1));

        TagID_Words_Map[tagID].insert(nl_string);

        #if SOVERLOAD
        lexicon.insert(g.translate(nl_string));
        #endif
    }

    #if SOVERLOAD
    g.inject_lexicon(lexicon);
    #endif


    // create a parser instance
    PARSER parser(g, tag_set, TagID_Words_Map);

    // parse all sentences
    for (auto s = sentences.begin(); s != sentences.end(); ++s)
    {
        if (verbosity > 1)
        {
            cout << "'" << helper::to_string(*s) << "'\n";
        }

        bool p = parser.parse(*s);

        if (verbosity > 2) parser.show_chart();
        if (verbosity > 1)
        {
            if(p) std::cout << "parse complete, input recognised.\n\n";
            else std::cout << "parse incomplete, input not recognised.\n\n";
        }
        else if (verbosity > 0) std::cout << p << std::endl;
    }

}
