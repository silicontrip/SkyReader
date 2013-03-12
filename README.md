SkyReader
=========

A cross platform (using hidapi) Skylander portal reader/editor/writer.

HIDAPI can be found here http://www.signal11.us/oss/hidapi/

    Usage:
    editor [-i <file>|-p] [-s <skylander>] [-d] [-e] [-o <file>|-P] [-M <money>] [-X experience] ... 

    Reading/Writing:
    -i <file>  read skylander data from file, with option to decrypt the data.
    -p		read skylander data from portal and decrypt the data.
    -s <skylander> select which skylander.
    -d		decrypt the data read from the file.
    -o <file>	write skylander data to <filename>.
    -P		encrypt and write skylander data to the portal.
    -e		encrypt data when writing file.
    -D		dump the data of a skylander to the display.
    -l		List skylanders on portal.

    Upgrade:
    -M <money>	upgrade skylander money (max 65,000).
    -X <xp>		upgrade skylander Experience (level 10 = 33,000).
    -H <hp>		upgrade skylander Hero Points (max 100).
    -C <challenges>	upgrade skylander challenges.
    -L <points>	upgrade the skylander skillpoints on the left path.
    -R <points>	upgrade the skylander skillpoints on the right path.
    -c		update checksums.

Examples
--------
        editor -p -o spyro.bin
This would save a copy of the figurine to the file spyro.bin

        editor -i spyro.bin -o spyro_upgrade.bin -L 65535 -M 65000 -X 33000 -H 100
upgrade spyro.bin using skills on the LEFT path seen in the character menu
and write it to file spyro_upgrade.bin

    editor -i spyro.bin -P -M 65000 -X 33000
Upgrade skylander, leave skills as is, and write to the portal.

    editor -i spyro.bin -P
Read file from spyro.bin and write it to the portal.
