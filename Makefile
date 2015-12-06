PROG = roboconfig
CFLAGS = -W -Wall -Idist -Isrc -pthread -g -O1 -std=c++11 -Wno-deprecated-register $(CFLAGS_EXTRA) 
SOURCES = $(PROG).cpp dist/mongoose.cpp dist/jsoncpp.cpp src/config.cpp src/server.cpp

$(PROG): $(SOURCES)
	$(CXX) -o $(PROG) $(SOURCES) $(CFLAGS)

clean:
	rm -rf $(PROG) *.exe *.dSYM *.obj *.exp .*o *.lib
