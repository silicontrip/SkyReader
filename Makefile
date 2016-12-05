

CFLAGS=-I/usr/local/include/libusb-1.0 -g
LDFLAGS=-L/usr/local/lib -lusb-1.0 -framework CoreFoundation -framework IOKit
OBJ= checksum.o fileio.o md5.o  rijndael.o crypt.o skylander.o main.o hid.o

LIBUSBIO = portalio_libusb.o
LIBHIDAPI = portalio_hidapi.o
WIN = portalio.o
IOKIT = portalio_iokit.o

editor: $(OBJ) $(LIBHIDAPI)
	g++ $(LDFLAGS) -o $@ $^	 

usbtest: usbtest.o
	gcc $(LDFLAGS) -o $@ $<

hid.o: hid.c
	gcc -c -o $@ $<

%.o:%.cpp
	g++ $(CFLAGS) -c $<

clean:
	rm -f *.o usbtest
