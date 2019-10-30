# Makefile for Linux
# likely also work under OS X
# Matthias Bisping


########################################################################
# variables
########################################################################

THIS_FILE := $(lastword $(MAKEFILE_LIST))

CMPL = g++
OPTS1 = -Wall -O3 -std=c++11

GRAMMARDEMO_CPP = src/grammardemo.cpp
GRAMMARDEMO_OUT = bin/grammardemo.out
PARSER_OUT = bin/parse.out
PARSER_SO_OUT = bin/parse_so.out
INDICATOR_DEMO_OUT = bin/indicator_demo.out

.DEFAULT_GOAL := default

########################################################################
# targets
########################################################################

default: $(PARSER_OUT)

# show help
help:

	@echo Usage:${\n}
	@echo make    ..................compiles src/parse.cpp
	@echo make    parserdemo1.......demonstrates parser
	@echo make    parserdemo2.......demonstrates parser
	@echo make    indicatordemo.....demonstrates indicator classes
	@echo make    grammardemo EXP...demonstrates grammar by reading in 10^EXP rules
	@echo make    docu..............generates documentation in doc
	@echo make    help..............shows this message
	@echo make    clean.............removes all generated files
	@echo make    complete_demo.....runs all demos


complete_demo: $(PARSER_OUT) $(GRAMMAR_OUT)

	@echo running indicatordemo
	@echo ---------------------
	@$(MAKE) -s -f $(THIS_FILE) indicatordemo

	@echo running grammardemo
	@echo -------------------
	@$(MAKE) -s -f $(THIS_FILE) grammardemo

	@echo running parserdemo1 -- toy grammar
	@echo ----------------------------------
	@$(MAKE) -s -f $(THIS_FILE) parserdemo1

	@echo running parserdemo2 -- overloaded grammar symbols
	@echo -------------------------------------------------
	@$(MAKE) -s -f $(THIS_FILE) parserdemo2

	@echo running parserdemo3 -- left recursive rules
	@echo -------------------------------------------
	@$(MAKE) -s -f $(THIS_FILE) parserdemo3

	@echo running parserdemo4 -- realistic grammar
	@echo ----------------------------------------
	@$(MAKE) -s -f $(THIS_FILE) parserdemo4

	@echo cleaning up
	@echo -----------
	@$(MAKE) -s -f $(THIS_FILE) clean


parserdemo1: $(PARSER_OUT) ## demo

	@./$(PARSER_OUT) -f data/example1.input -g data/example1.cfg -t data/example1.pos \
        -w data/example1.words -v 3


parserdemo2: $(PARSER_SO_OUT) ## demo

	@python src/genrules.py 3 gramrules
	@mv gramrules data/temp/
	@./$(PARSER_SO_OUT) -f data/example2.input -g data/temp/gramrules -t data/example2.pos \
        -w data/example2.words -v 2

parserdemo3: $(PARSER_OUT) ## demo

	@./$(PARSER_OUT) -f data/example4.input -g data/example4.cfg -t data/example4.pos \
        -w data/example4.words -v 2

parserdemo4: $(PARSER_OUT) ## demo

	@./$(PARSER_OUT) -f data/example1.input -g data/bitpar.cfg -t data/bitpar.pos \
        -w data/bitpar.words -v 2

indicatordemo: $(INDICATOR_DEMO_OUT) ##demonstrates busy indicators and progressbar

	@./bin/indicator_demo.out

$(INDICATOR_DEMO_OUT): incl/busy.hpp incl/load.hpp src/indicator_demo.cpp

	@$(CMPL) $(OPTS1) -o indicator_demo.out src/indicator_demo.cpp
	@mv indicator_demo.out bin


$(PARSER_OUT): incl/busy.hpp incl/chart.hpp incl/color.hpp incl/declarations.hpp \
               incl/grammar.hpp incl/helper.hpp incl/item.hpp incl/load.hpp \
               incl/parser.hpp incl/rule.hpp src/parse.cpp incl/translator.hpp

	@$(CMPL) $(OPTS1) -o parse.out src/parse.cpp
	@mv parse.out bin


$(PARSER_SO_OUT): incl/busy.hpp incl/chart.hpp incl/color.hpp incl/declarations.hpp \
               incl/grammar.hpp incl/helper.hpp incl/item.hpp incl/load.hpp \
               incl/parser.hpp incl/rule.hpp src/parse.cpp incl/translator.hpp

	@$(CMPL) $(OPTS1) -o parse_so.out -DSOVERLOAD=1 src/parse.cpp
	@mv parse_so.out bin


# make documentation
docu:

	@cd doc; rm -fr *
	@doxygen earley_parser.doxygen
	@mv html doc


# clean any files generated
clean_devel:

	@find . -name '*.o' -delete
	@find . -name '*.obj' -delete
	@find . -name '*.exe' -delete
	@find . -name '*.gch' -delete
	@find . -name '*.out' -delete
	@find . -name '*.png' -delete
	@find . -name '*.dat' -delete
	@cd incl; find . -type f ! -name "*.*" -delete
	@cd bin; rm -fr *
	@cd src; find . -type f ! -name "*.*" -delete
	@cd data/temp; rm -f *
	@cd doc; rm -fr *

clean:

	@find bin/ -type f -name "*" -delete
	@find data/temp/ -type f -name "*" -delete
	@find doc/ -type f -name "*" -delete

EXP = 6

# make grammardemo
$(GRAMMARDEMO_OUT):

	@$(CMPL) $(OPTS1) -o grammardemo.out $(GRAMMARDEMO_CPP)
	@mv grammardemo.out bin

grammardemo: src/genrules.py $(GRAMMARDEMO_OUT) ## generates 10^i rules and fills grammar with these

	@python src/genrules.py $(EXP) gramrules
	@mv gramrules data/temp
	@./$(GRAMMARDEMO_OUT)
	@rm data/temp/gramrules
