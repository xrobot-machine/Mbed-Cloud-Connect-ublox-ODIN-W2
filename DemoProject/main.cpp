/*
 * Example two
 * Provide a serial pass-through between the PC and an external UART.
 * https://os.mbed.com/docs/mbed-os/v5.15/apis/serial.html#serial-hello-world
 */

#include "mbed.h"
#include "platform/mbed_thread.h"

#define DEMO_FOR_UART 1
#define DEMO_FOR_RTOS_THREAD 1
#define DEMO_FOR_WIFI_LED 1

#ifdef DEMO_FOR_UART
Serial pc(USBTX, USBRX); // tx, rx
Serial device(MBED_CONF_APP_UART1_TX, MBED_CONF_APP_UART1_RX); // tx, rx
//void uart_passthrough_demo() {
//}
#endif

#if DEMO_FOR_RTOS_THREAD
#define WAIT_TIME_MS 100
Thread threadled1, threadled2, threadled3, threadled4;
DigitalOut led1(LED1, 1);
void led1_thread() {
    while (true) {
	led1 = !led1;
	thread_sleep_for(8*WAIT_TIME_MS);
    }
}
DigitalOut led2(LED2, 1);
void led2_thread() {
    while (true) {
	led2 = !led2;
	thread_sleep_for(4*WAIT_TIME_MS);
    }
}
DigitalOut led3(LED3, 1);
void led3_thread() {
    while (true) {
	led3 = !led3;
	thread_sleep_for(2*WAIT_TIME_MS);
    }
}
DigitalOut led4(LED4, 1);
void led4_thread() {
    while (true) {
	led4 = !led4;
	thread_sleep_for(WAIT_TIME_MS);
    }
}
DigitalOut led5(LED5, 1);
DigitalOut led6(LED6, 1);
DigitalOut led7(LED7, 1);

DigitalOut ledred(LED_RED, 1);
DigitalOut ledgreen(LED_GREEN, 1);
DigitalOut ledblue(LED_BLUE, 1);
volatile bool running = true;
// Blink function toggles the led in a long running loop
void blink(DigitalOut *led) {
    while (running) {
        *led = !*led;
        thread_sleep_for(100);
    }
}
#endif

#if DEMO_FOR_WIFI_LED
Thread threadwifi;
WiFiInterface *wifi;
#define WIFI_SCAN 0
const char *sec2str(nsapi_security_t sec)
{
    switch (sec) {
        case NSAPI_SECURITY_NONE:
            return "None";
        case NSAPI_SECURITY_WEP:
            return "WEP";
        case NSAPI_SECURITY_WPA:
            return "WPA";
        case NSAPI_SECURITY_WPA2:
            return "WPA2";
        case NSAPI_SECURITY_WPA_WPA2:
            return "WPA/WPA2";
        case NSAPI_SECURITY_UNKNOWN:
        default:
            return "Unknown";
    }
}
int wifi_scan(WiFiInterface *wifi)
{
    WiFiAccessPoint *ap;

    printf("WiFi Scan:\n");
    int count = wifi->scan(NULL,0);
    if (count <= 0) {
        printf("scan() failed with return value: %d\n", count);
        return 0;
    }

    /* Limit number of network arbitrary to 15 */
    count = count < 15 ? count : 15;
    ap = new WiFiAccessPoint[count];
    count = wifi->scan(ap, count);
    if (count <= 0) {
        printf("scan() failed with return value: %d\n", count);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\n", ap[i].get_ssid(),
               sec2str(ap[i].get_security()), ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
               ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5], ap[i].get_rssi(), ap[i].get_channel());
    }
    printf("%d networks available.\n", count);
    delete[] ap;
    return count;
}
void wifi_thread() {
    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        printf("ERROR: No WiFiInterface found.\n");
        //return -1;
    }

#if WIFI_SCAN
    int count = wifi_scan(wifi);
    if (count == 0) {
        printf("No WIFI APs found - can't continue further.\n");
        //return -1;
    }
#endif

    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error: %d\n", ret);
        //return -1;
    }

    led7.write(0);
    printf("Success\n\n");
    printf("MAC: %s\n", wifi->get_mac_address());
    printf("IP: %s\n", wifi->get_ip_address());
    printf("Netmask: %s\n", wifi->get_netmask());
    printf("Gateway: %s\n", wifi->get_gateway());
    printf("RSSI: %d\n\n", wifi->get_rssi());
    printf("\nDone\n");
    while (true) {
        led7 = !led7;
        ledgreen = !ledgreen;
        thread_sleep_for(WAIT_TIME_MS);
    }

    wifi->disconnect();
}
#endif

int main() {
#if DEMO_FOR_UART
    device.baud(9600);
    device.printf("Hello World!\n\r");
#endif

#if DEMO_FOR_RTOS_THREAD
    threadled1.start(led1_thread);
    threadled2.start(led2_thread);
    threadled3.start(led3_thread);
    //threadled4.start(led4_thread);
    threadled4.start(callback(blink, &led4));
#endif

#if DEMO_FOR_WIFI_LED
    threadwifi.start(wifi_thread);
#endif

#if DEMO_FOR_UART
    while (true) {
        //uart_passthrough_demo();
        if (pc.readable()) {
            device.putc(pc.getc());
        }
        if (device.readable()) {
            pc.putc(device.getc());
        }
    }
#endif
}
