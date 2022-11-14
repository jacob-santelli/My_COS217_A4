#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3
# dt* targets are built using checkerDT
# rules to build dtBad*.o and nodeBad*.o from source will fail
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

dt%: dynarray.o path.o checkerFT.o nodeFT%.o ft%.o ft_client_copy.o
	$(GCC) -g $^ -o $@

dynarray.o: dynarray.c dynarray.h
	$(GCC) -g -c $<

path.o: path.c path.h
	$(GCC) -g -c $<

ft_client_copy.o: ft_client.c ft.h a4def.h
	$(GCC) -g -c $<

checkerFT.o: checkerFT.c dynarray.h checkerFT.h nodeFT.h path.h a4def.h
	$(GCC) -g -c $<

nodeFTd.o: nodeFT.c dynarray.h checkerDT.h nodeFT.h path.h a4def.h
	$(GCC) -g -c $<

ft.o: ft.c dynarray.h checkerFT.h nodeDT.h ft.h path.h a4def.h
	$(GCC) -g -c $<

#You can't re-build the .o files we provide, and
#you shouldn't be changing the header files they rely on
#but in case the headers' modification times have changed,
#update the .o files' modification times to still be newer.
nodeDT%.o: dynarray.h checkerDT.h nodeDT.h path.h a4def.h
	touch $@

dtBad%.o: dynarray.h checkerDT.h nodeDT.h dt.h path.h a4def.h
	touch $@
