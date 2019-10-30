/**
 * @file busy.hpp
 * Classes for a bar that indicates a running process
 *
 * Matthias Bisping
 *
 * compilers: - clang-700.0.72
 *            - clang 3.5.2 / 3.5.0
 *            - GCC 5.2.0 / 4.8.3
 *            - Microsoft C/C++ 19.00.23026 for x86
 */

#ifndef __BUSY__HPP
#define __BUSY__HPP

#include "helper.hpp"
#include "declarations.hpp"

namespace BUSY
{
typedef COLOR::ColorString                                                   CS;
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                  BusyBar                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief can display a a bar indicating a busy process the CLI window
 */
class BusyBar
{

// update interval in calls
// speed therefore depends on CPU tick rate, system load and other factors
#define INTR 10000

////////////////////////////////////////////////////////////////////////////////
public:                                                     //    PUBLIC METHODS
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief sets the glyphs for the different parts of the indicator bar
     * @param s1c color for @p s1
     * @param s2c color for @p s2
     * @param s3c color for @p s3
     * @param str1 part of the indicator bar (specifics depend on derived class)
     * @param str2 part of the indicator bar (specifics depend on derived class)
     * @param str3 part of the indicator bar (specifics depend on derived class)
     */
    BusyBar(helper::CID s1c, helper::CID s2c, helper::CID s3c,
            sstr str1, sstr str2, sstr str3)
    :
    #ifdef UNIXLIKE
    s1(str1, s1c),
    s2(str2, s2c),
    s3(str3, s3c),
    #else
    s1("#"),
    s2("#"),
    s3("-"),
    #endif
    occupied(0),
    ocup(occupied, 1),
    ocdown(occupied, -1)
    {
        init();
    }
////////////////////////////////////////////////////////////////////////////////
    /// resets the line the indicator bar was running on
    void cancel(sost& o=std::cerr)
    {
        // do not display anything, if stdout is redirected to a file
        #ifdef _WIN32
        if (!_isatty(_fileno(stdout))) return;
        #else
        if (!isatty(fileno(stdout))) return;
        #endif
        blank_line(o);
        #ifdef UNIXLIKE
        o << "\e[?25h";
        #endif
        //tick = 0;
        //occupied = 0;
    }
////////////////////////////////////////////////////////////////////////////////
    /// updates the indicator bar
    template <typename T>
    inline void run(const T& cf , const T& cb,
                    const T& tfl, const T& tfr,
                    const T& tbl, const T& tbr,
                    sost& o=std::cerr)
    {
        // do not display anything, if stdout is redirected to a file
        #ifdef _WIN32
        if (!_isatty(_fileno(stdout))) return;
        #else
        if (!isatty(fileno(stdout))) return;
        #endif
        // if an interval is over, update
        if(tick == interval)
        {
            #ifdef UNIXLIKE
            o << "\e[?25l";
            #endif
            // if moving right
            if(right)
            {
                // test whether the is still capacity left for 1 more update
                if(capacity-occupied >= 1)
                {
                    // if so, update
                    next(tfl, cf, tfr, ocup);
                }
                // if not blank the line and reverse the direction
                else { blank_line(o); right = false; }
            }
            // if moving left
            if(!right)
            {
                // test whether the is still capacity left for 1 more update
                if(occupied-1 >= 0)
                {
                    // if so, update
                    next(tbl, cb, tbr, ocdown);
                }
                // if not blank the line and reverse the direction
                else
                {
                    blank_line(o); right = true;
                    next(tfl, cf, tfr, ocup);
                }
            }
            tick = 0;
        }
        ++tick;
    }
////////////////////////////////////////////////////////////////////////////////
private:                                                    //   PRIVATE METHODS
////////////////////////////////////////////////////////////////////////////////
    /// initializes fields
    void init()
    {
        // query the terminal column count
        cls = helper::get_terminal_columns();
        if (cls == -1) cls = 30;

        s1Size = helper::utf8_size(s1);
        s2Size = helper::utf8_size(s2);
        s3Size = helper::utf8_size(s3);

        // normalise speed on window width (column number)
        interval = INTR*((double)s3Size/cls);
        // interval/((double)s3Size/cls) same value in all terminals

        right = true;
        tick = 0;
    }
////////////////////////////////////////////////////////////////////////////////
protected:                                                  // PROTECTED METHODS
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief calculates the number of @p js that fit on one line together with
     *        one @p i. Returns false if less than 1 j fits on the line.
     * @param j some literal
     * @param i soem literal
     */
    template <typename T>
    bool calc_cap(T i, T j)
    {
        capacity = (int)(cls-i)/j;
        if (capacity == 0) return false;
        return true;
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief fills line incrementally with pattern s1* s2 s3*. Per call the
     *        line is updated, advancing the position of all moving elements
     *        of the indicator bar.
     * @tparam T the type of the literal arguments
     * @param s1 some literal
     * @param s2 some literal
     * @param s3 some literal
     * @param incr \b Increment functor
     * @param o stream to send to
     */
    template <typename T>
    void next(const T& s1,
              const T& s2,
              const T& s3,
              helper::Increment<int>& incr,
              sost& o=std::cerr)
    {
        // send left par of bar
        for(int i = 0; i < occupied; ++i)
        {
            o << s1;
        }
        // send head
        o << s2;
        // send right part of bar
        for(int i = occupied; i < capacity; ++i)
        {
            o << s3;
        }
        o << "\r";
        o.flush();
        // increment/ decrement \b occupied through the interface of the functor
        // @p incr. Depending on which functor was passed, occupied will be
        // incremented or decremented.
        ++incr;
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief fills line with literal @p sym
     * @tparam T type of the literal
     * @param sym some literal
     * @pre @p needs to have string conversion defined
     * @param o stream to send to
     */
    template <typename T>
    void fill_line(T& sym, sost& o=std::cerr) const
    {
        o << "\r";
        // get the number of glyphs in @p sym and fill the line accordingly
        // (getting the number of chars rather than glyphs  would potentially
        // spill over into the next line)
        for (int i = 0; i < cls/helper::utf8_size(sym); ++i)
        {
            o << sym;
        }
        o << "\r";
        o.flush();
    }
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief fills line with whitespace
     * @param o stream to send to
     */
    void blank_line(sost& o=std::cerr) const
    {
        o << "\r";
        for (int i = 0; i < cls; ++i)
        {
            o << " ";
        }
        o << "\r";
        o.flush();
    }
////////////////////////////////////////////////////////////////////////////////
protected:                                                  //  PROTECTED FIELDS
////////////////////////////////////////////////////////////////////////////////

    #ifdef _WIN32
    const sstr s1;                 ///< bar element
    const sstr s2;                 ///< bar element
    const sstr s3;                 ///< bar element
    #else
    const CS s1;                   ///< bar element
    const CS s2;                   ///< bar element
    const CS s3;                   ///< bar element
    #endif

    int s1Size;                    ///< size of the \b s1 literal
    int s2Size;                    ///< size of the \b s2 literal
    int s3Size;                    ///< size of the \b s3 literal

    int cls;                       ///< number of columns
    int capacity;                  ///< line capacity for specifc string
    int occupied;                  ///< used up capacity counting from the left
    unsigned long tick;            ///< calls since last display update
    unsigned long long interval;   ///< update interval in function calls
    bool right;

    helper::Increment<int> ocup;   ///< handle for occupied variable; increments
    helper::Increment<int> ocdown; ///< handle for occupied variable; decrements
////////////////////////////////////////////////////////////////////////////////
};
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                   Variant1                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Variant with a changing moving head literal and a filler literal
 *        the first version of the head moves left, the other right. The
 *        filler covers the rest of the line
 */
class Variant1 : public BusyBar
{
////////////////////////////////////////////////////////////////////////////////
public:                                                     //   PUBCLIC METHODS
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief sets the glyphs for the different parts of the indicator bar
     * @param s1c color for @p s1
     * @param s2c color for @p s2
     * @param s3c color for @p s3
     * @param s1 left moving head
     * @param s2 right moving head
     * @param s3 line filler
     */
    Variant1(helper::CID s1c = helper::CID::RED,
              helper::CID s2c = helper::CID::GREEN,
              helper::CID s3c = helper::CID::BLACK,
              sstr s1=" ◀ ",   // ◯ ▬▬ ▰▰▰ ◫ ◻ ⬡ ♽ ███ ▫ lhead lstat
              sstr s2=" ▶ ",   // ◉ ▬▬ ▰▰▰ ◯ ◻ ⌬ ♼ ███ ▢ rhead rstat
              sstr s3=" ● ")   // ◯ ▬▬ ▰▰▰ ◫ ◻ ⬡ ♽ ███ ▫ trail mov
    : BusyBar(s1c,s2c,s3c,s1,s2,s3)
    {
        init();
    }
////////////////////////////////////////////////////////////////////////////////
    inline virtual void run(sost& o=std::cerr)
    {
        BusyBar::run(s2,s1,s3,s3,s3,s3);
    }
////////////////////////////////////////////////////////////////////////////////
private:                                                    //   PRIVATE METHODS
////////////////////////////////////////////////////////////////////////////////
    /// initializes fields
    void init()
    {
        // s1 and s2 need to be same size, because they substitute each
        // other depening on the heading
        if(s1Size != s2Size)
        {
            helper::msg("error:",
            "paired symbols s1 and s2 require same number of glyphs.",
            __FILE__, __LINE__);
            abort();
        }

        // calculate the number of s3s that fit in a line together
        // with an s1 respectively s2, as there is  only ever one s1
        // resp. s2 on the line
        if(!calc_cap(s1Size, s3Size))
        {
            helper::msg("error:",
            "s1 and s3 combined exceed line capacity.",
            __FILE__, __LINE__);
            abort();
        }
    }
////////////////////////////////////////////////////////////////////////////////
};
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                   Variant2                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
/**
 *@brief Variant with one unchanging moving head, a left filler and a right
 *       filler. The head moves over the lines back and forth, the fillers
 *       cover either side of it.
 */
class Variant2 : public BusyBar
{
////////////////////////////////////////////////////////////////////////////////
public:                                                     //    PUBLIC METHODS
////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief sets the glyphs for the different parts of the indicator bar
     * @param s1c color for @p s1
     * @param s2c color for @p s2
     * @param s3c color for @p s3
     * @param s1 left filler
     * @param s2 moving head
     * @param s3 right filler
     */
    Variant2(helper::CID s1c = helper::CID::GREEN,
            helper::CID s2c = helper::CID::RED,
            helper::CID s3c = helper::CID::GREEN,
            sstr s1=" ◉ ",   // ◯ ▬▬ ▰▰▰ ◫ ◻ ⬡ ♽ ███ ▫ head
            sstr s2=" ◉ ",   // ◉ ▬▬ ▰▰▰ ◯ ◻ ⌬ ♼ ███ ▢ lfill
            sstr s3=" ◯ ")   // ◯ ▬▬ ▰▰▰ ◫ ◻ ⬡ ♽ ███ ▫ rfill
    : BusyBar(s1c,s2c,s3c,s1,s2,s3)
    {
        init();
    }
////////////////////////////////////////////////////////////////////////////////
    inline virtual void run(sost& o=std::cerr)
    {
        BusyBar::run(s2,s2,s1,s3,s1,s3);
    }
////////////////////////////////////////////////////////////////////////////////
private:                                                    //   PRIVATE METHODS
////////////////////////////////////////////////////////////////////////////////
    /// initialises fields
    void init()
    {
        // s1 and s3 need to be same size, because they substitute each
        // other depening on the heading
        if(s1Size != s3Size)
        {
            helper::msg("error:",
            "paired symbols s1 and s3 require same number of glyphs.",
            __FILE__, __LINE__);
            abort();
        }

        // calculate the number of s1s that fit in a line together
        // with an s1 respectively s3, as there is  only ever one
        // s2 on the line
        if(!calc_cap(s2Size, s1Size))
        {
            helper::msg("error:",
            "s1 and s2 combined exceed line capacity.",
            __FILE__, __LINE__);
            abort();
        }

    }
////////////////////////////////////////////////////////////////////////////////
};

} // BUSY

#endif // __BUSY__HPP
