.PHONY: test test-js test-native esp32 nrf52 brooch all clean

test: test-native test-js

test-native:
	pio run -e native_test
	.pio/build/native_test/program

test-native-gcc:
	gcc -std=c99 -Wall -I shared -o /tmp/vb_test \
		test/native/test_protocol.c shared/vb_protocol.c shared/vb_button.c
	/tmp/vb_test

test-js:
	node --test test/js/protocol.test.mjs

esp32:
	pio run -d esp32 -e esp32_brooch

nrf52:
	pio run -e nrf52840_brooch

brooch: nrf52

all: test esp32 nrf52

clean:
	pio run -t clean
	pio run -d esp32 -t clean
