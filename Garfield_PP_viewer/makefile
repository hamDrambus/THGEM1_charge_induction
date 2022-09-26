INCDIR = $(GARFIELD_HOME)/include/Garfield
LIBDIR = $(GARFIELD_HOME)/lib
TARNAME = ChargeAvalanche_v00

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

target: $(TARNAME).o
	$(CXX) $(TARNAME).o -o $(TARNAME) $(LDFLAGS)

$(TARNAME).o: $(TARNAME).cc
	$(CXX) $(CFLAGS) $(TARNAME).cc

clean:
	rm -f $(TARNAME).o $(TARNAME)

