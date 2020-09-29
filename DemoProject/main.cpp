/*
 * Example two
 * Provide a serial pass-through between the PC and an external UART.
 * https://os.mbed.com/docs/mbed-os/v5.15/apis/serial.html#serial-hello-world
 */

#include "mbed.h"

#define DEMO_FOR_UART 1

#if DEMO_FOR_UART
Serial pc(USBTX, USBRX); // tx, rx
Serial device(MBED_CONF_APP_UART1_TX, MBED_CONF_APP_UART1_RX); // tx, rx
#endif

int main() {

#if DEMO_FOR_UART
	device.baud(9600);
	device.printf("Hello World!\n\r");
#endif
	while (true) {
#if DEMO_FOR_UART
		if (pc.readable()) {
			device.putc(pc.getc());
		}
		if (device.readable()) {
			pc.putc(device.getc());
		}
#endif
	}
}
