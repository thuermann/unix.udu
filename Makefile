#
# $Id: Makefile,v 1.1 2012/10/15 21:25:27 urs Exp $
#

RM       = rm -f

programs = udu

.PHONY: all
all: $(programs)

.PHONY: clean
clean:
	$(RM) $(programs) *.o core
