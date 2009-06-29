
# Pipeline objects and flags
OBJ=pipe2.o
TARGET=pipe2
CPPFLAGS=-O3 

# Spice location and flags
DEFAULT_SPICE=../cspice
SPICE_LIBS=-L$(SPICE)/lib -lcspice
SPICE_INC=-I$(SPICE)/include

# Use a svn revision number or build timestamp for log output
ifndef REVISION
REVISION=$(strip $(shell (svn info 2>/dev/null || echo "Revision: " $$(date +s%s) )| grep Revision: | cut -f2 -d:))
endif
CPPFLAGS+=-DREVISION='"${REVISION}"'

# Check if our DEFAULT_SPICE directory exists, and if so use that
ifndef SPICE
ifeq ($(shell test -d ${DEFAULT_SPICE}; echo $$?),0)
SPICE=${DEFAULT_SPICE}
endif
endif

# Warn if we can't find the Spice directory
ifdef SPICE
LIBS+=${SPICE_LIBS}
CPPFLAGS+=${SPICE_INC} -DHAVE_SPICE
else
$(info ---------------------------------------------)
$(warning "Warning: SPICE is not defined")
$(info ---------------------------------------------)
endif

# Pipeline targets (all, pipe2, and clean)
all: $(TARGET)

$(OBJ): crater/*.hpp

$(TARGET): $(OBJ)
	$(CXX) -o $(TARGET) $(OBJ) $(LIBS)

geo: geo.o
	$(CC) -o geo geo.o $(LIBS) -lm

clean:
	-rm $(OBJ)
