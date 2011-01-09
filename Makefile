run : mosaic.cpp
	g++ -lcv -lcxcore -lhighgui $^ -o $@
clean: 
	rm ./run
