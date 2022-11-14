#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3
# ft* targets are built using checkerFT
# rules to build ftBad*.o and nodeBad*.o from source will fail
# Author: Christopher Moretti
#--------------------------------------------------------------------

GCC = gcc217
#GCC = gcc217m

TARGETS = ft

.PRECIOUS: %.o

all: $(TARGETS)

clean:
	rm -f $(TARGETS) meminfo*.out

clobber: clean
	rm -f dynarray.o path.o ft_client_copy.o checkerFT.o nodeFT.o ft.o *~

ft: dynarray.o path.o checkerFT.o nodeFT.o ft.o ft_client_copy.o
	$(GCC) -g $^ -o $@

dynarray.o: dynarray.c dynarray.h
	$(GCC) -g -c $<

path.o: path.c path.h
	$(GCC) -g -c $<

ft_client_copy.o: ft_client_copy.c ft.h a4def.h
	$(GCC) -g -c $<

checkerFT.o: checkerFT.c dynarray.h checkerFT.h nodeFT.h path.h a4def.h
	$(GCC) -g -c $<

nodeFT.o: nodeFT.c dynarray.h checkerFT.h nodeFT.h path.h a4def.h
	$(GCC) -g -c $<

ft.o: ft.c dynarray.h checkerFT.h nodeFT.h ft.h path.h a4def.h
	$(GCC) -g -c $<

