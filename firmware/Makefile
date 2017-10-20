all:
	    platformio -f -c vim run

upload:
	    platformio -f -c vim run --target upload --verbose

clean:
	    platformio -f -c vim run --target clean

program:
	    platformio -f -c vim run --target program

uploadfs:
	    platformio -f -c vim run --target uploadfs

update:
	    platformio -f -c vim update

monitor:
	    platformio device monitor --baud=57600 --raw --port=/dev/cu.wchusbserial14240 --eol=CRLF
