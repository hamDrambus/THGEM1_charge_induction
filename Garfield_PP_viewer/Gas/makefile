INCDIR = $(GARFIELD_HOME)/include/Garfield
LIBDIR = $(GARFIELD_HOME)/lib
TARNAME1 = CalcGas
TARNAME2 = ViewGas

# Compiler flags
CFLAGS = -Wall -Wextra -Wno-long-long \
	`root-config --cflags` \
	-O3 -fno-common -c \
	-I$(INCDIR)

# Debug flags
#CFLAGS += -g

LDFLAGS = -L$(LIBDIR) -lGarfield
LDFLAGS += `root-config --glibs` `root-config --ldflags` -lGeom \
	-lgfortran -lm

#LDFLAGS += -g

target: $(TARNAME1).o $(TARNAME2).o
	$(CXX) $(TARNAME1).o -o $(TARNAME1) $(LDFLAGS)
	$(CXX) $(TARNAME2).o -o $(TARNAME2) $(LDFLAGS)
	rm -f $(TARNAME1).o $(TARNAME2).o

$(TARNAME1).o: $(TARNAME1).cc
	$(CXX) $(CFLAGS) $(TARNAME1).cc

$(TARNAME2).o: $(TARNAME2).cc
	$(CXX) $(CFLAGS) $(TARNAME2).cc

clean:
	rm -f $(TARNAME1).o $(TARNAME1)
	rm -f $(TARNAME2).o $(TARNAME2)

