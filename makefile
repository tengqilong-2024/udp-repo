## version 4
CXX =g++
TARGET =main
#all cpp
SRC = $(wildcard *.cpp)
#all obj
OBJ = $(patsubst %.cpp, %.o,$(SRC) -lpthread)
#OBJ = $(patsubst %.cpp, %.o,$(SRC))
# 编译选项
CXXFLASS = -c -Wall
# TARGET:$@;OBJ $^
$(TARGET): $(OBJ)
	$(CXX) -o $@ $^
%.o: %.cpp
	$(CXX) $(CXXFLASS) $< -o $@

.PHONY: clean
# liunx del->rm
clean:
	rm -f *.o $(TARGET) 

# # version 2 g++
# CXX = g++
# TARGET = main
# OBJ = main.o Thrust.o Sbus.o ntrip_util.o Gnss.o Contrl.o
# $(TARGET): $(OBJ)
# 	$(CXX) -o $(TARGET) $(OBJ)
# main.o: main.cpp
# 	$(CXX) -c main.cpp

# Thrust.o: Thrust.cpp
# 	$(CXX) -c Thrust.cpp

# Sbus.o: Sbus.cpp
# 	$(CXX) -c Sbus.cpp

# ntrip_util.o: ntrip_util.cpp
# 	$(CXX) -c ntrip_util.cpp

# Gnss.o: Gnss.cpp
# 	$(CXX) -c  Gnss.cpp

# Contrl.o: Contrl.cpp
# 	$(CXX) -c Contrl.cpp

# .PHONY: clean
# # liunx del->rm
# clean:
# 	rm -f *.o $(TARGET) 
# gnss:
# 	$(CXX) -c -o thread -lpthread Gnss.cpp