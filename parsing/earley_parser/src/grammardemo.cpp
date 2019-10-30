#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma warning(disable : 4503)
#endif

#include <iostream>
#include <chrono>

#include "../incl/rule.hpp"

using namespace std;
using namespace std::chrono;



int main()
{
    std::ifstream gr("data/temp/gramrules");

    if(!gr) exit(1);

    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    typedef string ES;
    typedef long IS;
    typedef Earley::CFGRuleParser<IS, ES>          RP;
    typedef Earley::CFGValidator<RP::ES>           V;
    typedef Earley::Grammar<V, RP>                 GRAMMAR;

    GRAMMAR g(gr);

    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();

    long int elapsed = duration/(float)1000;

    std::cout << "rules processed in " << elapsed << " milliseconds\n";
}
