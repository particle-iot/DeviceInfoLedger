

all : ConfigTest
	./ConfigTest

ConfigTest : ConfigTest.cpp ../src/DeviceInfoLedger.cpp ../src/DeviceInfoLedger.h libwiringgcc
	gcc ConfigTest.cpp ../src/DeviceInfoLedger.cpp UnitTestLib/libwiringgcc.a -std=c++17 -lc++ -IUnitTestLib -I../src -I. -o ConfigTest -DUNITTEST

check : ConfigTest.cpp  ../src/DeviceInfoLedger.cpp ../src/DeviceInfoLedger.h libwiringgcc
	gcc ConfigTest.cpp ../src/DeviceInfoLedger.cpp UnitTestLib/libwiringgcc.a -g -O0 -std=c++11 -lc++ -IUnitTestLib -I ../src -o ConfigTest && valgrind --leak-check=yes ./ConfigTest 

libwiringgcc :
	cd UnitTestLib && make libwiringgcc.a 	
	
.PHONY: libwiringgcc