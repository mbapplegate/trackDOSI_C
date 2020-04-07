IDIR=include
IOPENCVDIR=/home/matthew/opencv/installation/openCV-master/include/opencv4/
LIBDIR=/home/matthew/opencv/installation/openCV-master/lib/
LIBS=opencv_core opencv_highgui opencv_imgproc opencv_videoio

LINKFLAGS = $(addprefix -l,$(LIBS))

CC=g++
SRCDIR := src
BUILDDIR := build
TARGET := bin/trackDOSIC

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g -Wall

INC := -I$(IDIR) -I$(IOPENCVDIR)
LIB := -L$(LIBDIR)

$(TARGET) : $(OBJECTS)
	@echo " Linking. . ."
	@echo " $(CC) $^ trackDOSIC.cxx -o $(TARGET) $(LIB) $(LINKFLAGS) $(INC)"; $(CC) $^ trackDOSIC.cxx -o $(TARGET) $(LIB) $(LINKFLAGS) $(INC)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) -c $(CFLAGS) $(INC) $(LIB) $(LINKFLAGS) -o $@ $<";  $(CC) -c $(CFLAGS) $(INC) $(LIB) $(LINKFLAGS) -o $@ $<

clean:
	@echo " Cleaning. . .";
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

testing: $(OBJECTS)
	$(CC) $^ $(CFLAGS) testing/testASCData.cxx $(INC) $(LIB) $(LINKFLAGS) -o bin/testASCData

.PHONY: clean testing
