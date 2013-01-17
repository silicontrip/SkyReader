

OBJ= checksum.o fileio.o md5.o  rijndael.o crypt.o skylander.o

all: $(OBJ)

%.o:%.cpp
	gcc -c $<
