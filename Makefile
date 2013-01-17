
CFLAGS=-I/opt/local/include/libusb-1.0
LDFLAGS=-L/opt/local/lib -lusb-1.0
OBJ= checksum.o fileio.o md5.o  rijndael.o crypt.o skylander.o \
	usbtest.o



usbtest: usbtest.o
	gcc $(LDFLAGS) -o usbtest $<

objects: $(OBJ)

%.o:%.cpp
	gcc $(CFLAGS) -c $<
