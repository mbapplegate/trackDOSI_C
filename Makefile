IDIR=include
IOPENCVDIR=/home/matthew/opencv/installation/openCV-master/include/opencv4/
IBOOSTDIR=/usr/include/boost/
LIBDIR_OCV=/home/matthew/opencv/installation/openCV-master/lib/
LIBDIR_BOOST=/usr/lib/
LIBS=opencv_core opencv_highgui opencv_imgproc opencv_videoio boost_system boost_filesystem stdc++

LINKFLAGS = $(addprefix -l,$(LIBS))

CC=g++
SRCDIR := src
BUILDDIR := build
TARGET := bin/trackDOSIC.out

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g -Wall -std=c++11

INC := -I$(IDIR) -I$(IOPENCVDIR) -I$(IBOOSTDIR)
LIB := -L$(LIBDIR_OCV) -L$(LIBDIR_BOOST)

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
	$(CC) $^ $(CFLAGS) testing/testASCData.cxx $(INC) $(LIB) $(LINKFLAGS) -o bin/testASCData.out

testFDPM: $(OBJECTS)
	$(CC) $^ $(CFLAGS) testing/testFDPM.cxx $(INC) $(LIB) $(LINKFLAGS) -o bin/testFDPM.out

testopencv: $(OBJECTS)
	$(CC) $^ $(CFLAGS) testing/testOpenCV.cxx $(INC) $(LIB) $(LINKFLAGS) -o bin/testOpenCV.out

.PHONY: clean testing testFDPM
