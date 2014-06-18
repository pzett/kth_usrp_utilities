#==============================================================
CXXFLAGS = -o3 -Wall -DNDEBUG
LDLIBS = -luhd -lboost_program_options

#==============================================================
 
all:	tx rx tx_60GHz harness rx_60GHz tx_60GHz_MIMO rx_60GHz_MIMO

#==============================================================
clean:
	rm -f *.o 
	rm -f tx rx tx_60GHz harness rx_60GHz
#==============================================================
rx: rx.o 
	$(CXX) -o $@ $^ $(LDLIBS) 
#==============================================================
tx: tx.o 
	$(CXX) -o $@ $^ $(LDLIBS) 
#==============================================================

tx_60GHz: tx_60GHz.o board_60GHz.o board_60GHz.hpp 
	$(CXX) -o $@ $^ $(LDLIBS) 
#==============================================================
rx_60GHz: rx_60GHz.o board_60GHz.o board_60GHz.hpp
	$(CXX) -o $@ $^ $(LDLIBS) 
#==============================================================
tx_60GHz_MIMO: tx_60GHz_MIMO.o board_60GHz.o board_60GHz.hpp 
	$(CXX) -o $@ $^ $(LDLIBS) 
#==============================================================
rx_60GHz_MIMO: rx_60GHz_MIMO.o board_60GHz.o board_60GHz.hpp 
	$(CXX) -o $@ $^ $(LDLIBS) 
#==============================================================
rx_multi_samples: rx_multi_samples.o board_60GHz.o board_60GHz.hpp 
	$(CXX) -o $@ $^ $(LDLIBS) 
#==============================================================




square_elements_of_array_debug.cpp:	square_elements_of_array.cpp
	cp square_elements_of_array.cpp square_elements_of_array_debug.cpp
square_elements_of_array_debug.o:	CXXFLAGS = -ggdb -Wall
harness.o:				CXXFLAGS = -ggdb -Wall
harness:	harness.o	square_elements_of_array_debug.o 
	$(CXX) -o $@ $^ $(LDLIBS) 

