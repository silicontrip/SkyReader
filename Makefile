

CFLAGS=-I/usr/include/libusb-1.0 -g -pthread
LDFLAGS= -lusb-1.0 -lrt -pthread
OBJ= checksum.o fileio.o md5.o  rijndael.o crypt.o skylander.o main.o hid.o

LIBUSBIO = portalio_libusb.o
LIBHIDAPI = portalio_hidapi.o
WIN = portalio.o
IOKIT = portalio_iokit.o

editor: $(OBJ) $(LIBHIDAPI)
	g++ -o $@ $^	  $(LDFLAGS)

usbtest: usbtest.o
	gcc $(LDFLAGS) -o $@ $<

hid.o: hid.c
	gcc $(CFLAGS) -c -o $@ $<

%.o:%.cpp
	g++ $(CFLAGS) -c $<

clean:
	rm -f *.o usbtest
