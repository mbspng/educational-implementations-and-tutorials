/**
 * @file load.hpp
 * Class for progress bar functionality
 *
 * Matthias Bisping
 *
 * compilers: - clang-700.0.72
 *            - clang 3.5.2 / 3.5.0
 *            - GCC 5.2.0 / 4.8.3
 *            - Microsoft C/C++ 19.00.23026 for x86
 */

#ifndef __LOAD__HPP
#define __LOAD__HPP

#include "declarations.hpp"
#include "helper.hpp"
#ifdef UNIXLIKE
#include <unistd.h>
#endif
#include <algorithm>
#include <math.h>

namespace LOAD
{
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                 Progressbar                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief can display a progress bar in the CLI window
 */
class Progressbar
{
#define PSD 8 // size of percentage display
////////////////////////////////////////////////////////////////////////////////
public:                                                    //    PUBLIC TYPEDEFS
////////////////////////////////////////////////////////////////////////////////
typedef COLOR::ColorString                                                   CS;
typedef COLOR::CID                                                          CID;
typedef std::string                                                         sym;
typedef unsigned long                                                      step;
typedef std::vector<sym>                                                 symvec;
typedef std::vector<CS>                                                 csymvec;
////////////////////////////////////////////////////////////////////////////////
public:                                                    //     PUBLIC METHODS
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief initializes Progressbar with the maximum number of steps
     *        \p needed to finish the task the progress of which the
     *        progress bar is supposed to display.
     * @param max number of steps after which 100% is to be reached
     * @param barcolor the color of the individual bars
     * @param bar the glyph for the individual bars
     * @param pre the glyph for the progress bar background
     */
    Progressbar(step max, CID barcolor=CID::GREEN,
                sym bar="▓", sym pre= "░") // ▓ ◉
    :max(max),
    #ifdef UNIXLIKE
    bar(bar, barcolor),
    lbracket(""),
    rbracket(""),
    pre(pre) // ░ ◯
    #else
    bar("#"),
    lbracket("["),
    rbracket("]"),
    pre("-")
    #endif
    {
        init();
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief advances the progress bar every time an interval of x
     *        function calls has been completed
     * @param current the current iteration
     * @param o the stream to write to
     */
    inline void run(step current, sost& o=std::cerr)
    {
        // do not display anything, if stdout is redirected to a file
        #ifdef _WIN32
        if (!_isatty(_fileno(stdout))) return;
        #else
        if (!isatty(fileno(stdout))) return;
        #endif
        if(current%interval == 0 && !(current > max))
        {
            // do away with the cursor
            #ifdef UNIXLIKE
            o << "\e[?25l";
            #endif

            // fill line with background symbol
            for (int i = 0; i < PSD+lbracketSize; ++i) o << " ";
            for (unsigned i = 0; i < pres; ++i) o << pre;
            o << rbracket << "\r";
            o.flush();

            // calculate percentage and make a string of it
            float prct = ((float)current/max)*100;
            sstr strprct = helper::to_string((int)prct);
            // get percantge to length of three
            if (helper::utf8_size(strprct) < 2) strprct = " "+strprct;
            if (helper::utf8_size(strprct) < 3) strprct = " "+strprct;

            // print percentage and bar
            o << "[" << strprct << "% ] " << lbracket;
            for (auto i = pbar.begin(); i != pbar.end(); ++i) o << *i;

            o << "\r";
            o.flush();

            pbar.push_back(bar);
        }
        if(current>=max-1)
        {
            cancel();
        }
    }
////////////////////////////////////////////////////////////////////////////////
private:                                                   //    PRIVATE METHODS
////////////////////////////////////////////////////////////////////////////////
    /// initialises fields
    void init()
    {
        if (max == 0)
        {
            helper::msg("error:","invalid range (0,0]", __FILE__, __LINE__);
            exit(1);
        }

        barsize = helper::utf8_size(bar);
        lbracketSize = helper::utf8_size(lbracket);
        rbracketSize = helper::utf8_size(rbracket);

        cls = helper::get_terminal_columns();
        if (cls == -1) cls = 30;
        cls-=(PSD+lbracketSize+rbracketSize); // columns - "DDD% []"

        // it requires updates updates to fill the line with bars of
        // size barsize
        updates = cls/barsize;
        // same goes for pre
        pres = cls/helper::utf8_size(pre);

        // if max is smaller than updates, update as many times as
        // possible, by setting updates = max
        // then concatenate bar f times with itself, so the full
        // line can still be filled despite the lower update count
        if (max <= updates)
        {
            updates = max;
            int f = cls/(barsize*updates);

            #ifdef _WIN32
            sstr temp_bar = bar;
            for (int i = 0; i < f-1; ++i) bar+=temp_bar;
            #else
            sstr temp_bar = bar.str;
            for (int i = 0; i < f-1; ++i) bar.str+=temp_bar;
            #endif
        }

        pbar.reserve(updates);

        // Recalculate max, so it is an integer multiple of updates.
        // This way an interval can be an integer and updates*interval
        // be exactly equal to max.
        // This introduces a slight inaccuracy of the progress bar and
        // causes it to reach 100% a little too early, but assures
        // overall consistency regardless of the initial size of max.
        // The amount of time 100% is reached early is strictly less
        // than "update" steps, as the discrepancy between max and an
        // integer multiple of updates closest to max had to be less
        // than the value of updates, otherwise max would have been an
        // exact multiple of updates.
        // The delay therefore has a fix maximum.
        max = floor((double)max/updates)*updates;

        interval = ((float)max/updates);
    }
////////////////////////////////////////////////////////////////////////////////
/// blanks the current line and brings back the cursor
void cancel(sost& o=std::cerr) const
    {
        blank_line(o);
        #ifdef UNIXLIKE
        o << "\e[?25h";
        #endif
    }
////////////////////////////////////////////////////////////////////////////////
/// blanks the current line by filling it with spaces
void blank_line(sost& o=std::cerr) const
    {
        o << "\r";
        for (int i = 0; i < cls+PSD+lbracketSize+rbracketSize; ++i)
        {
            o << " ";
        }
        o << "\r";
        o.flush();
    }
////////////////////////////////////////////////////////////////////////////////
private:                                                   //     PRIVATE FIELDS
////////////////////////////////////////////////////////////////////////////////
    step max;
    #ifdef UNIXLIKE
    CS bar;           ///< progress bar glyph
    csymvec pbar;     ///< progress bar
    CS lbracket;      ///< left bracket
    CS rbracket;      ///< right bracket
    CS pre;           ///< "background" glyph; fills progress bar
    #elif defined _WIN32
    sstr bar;         ///< progress bar glyph
    symvec pbar;      ///< progress bar
    sstr lbracket;    ///< left bracket
    sstr rbracket;    ///< right bracket
    sstr pre;         ///< "background" glyph; fills progress bar
    #else
        #error "unkown platform"
    #endif

    step interval;    ///< the number of function calls between updates
    unsigned updates; ///< the total of updates to be made
    unsigned pres;    ///< number of background glyphs required to fill line
    int cls;          ///< the number of columns available for the progress bar
    int barsize;      ///< the size of a single bar literal
    int lbracketSize; ///< size of left bracket literal
    int rbracketSize; ///< size of right bracket literal
}; // Progressbar

} // LOAD

#endif // __LOAD__HPP
