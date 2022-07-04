SRC_FILES = $(shell find ./ -name '*.cpp')
OUTPUT= rvcdiss

compile:	$(SRC_FILES)
	g++ $(SRC_FILES) -o $(OUTPUT)


CTest:
	g++ main.cpp compressed.cpp ThirtyTwoBit.cpp 32_bit.cpp -o $(OUTPUT)

ATest:
	g++ main.cpp ThirtyTwoBit.cpp 32_bit.cpp compressed.cpp -o $(OUTPUT)

ctest:
	g++ main1.cpp compressed.cpp -o $(OUTPUT)

clean:
	$(RM) $(OUTPUT)
