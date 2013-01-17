

CFLAGS=-I/opt/local/include/libusb-1.0 -g
LDFLAGS=-L/opt/local/lib -lusb-1.0
OBJ= checksum.o fileio.o md5.o  rijndael.o crypt.o skylander.o \
	 portalio_libusb.o main.o

editor: $(OBJ)
	g++ $(LDFLAGS) -o editor $(OBJ)	

usbtest: usbtest.o
	gcc $(LDFLAGS) -o usbtest $<


%.o:%.cpp
	g++ $(CFLAGS) -c $<

clean:
	rm -f *.o usbtest
