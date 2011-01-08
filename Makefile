run : mosaic.cpp
	g++ -lcv -lcxcore -lhighgui $^ -o $@
