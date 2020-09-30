/*
 * Example two
 * Provide a serial pass-through between the PC and an external UART.
 * https://os.mbed.com/docs/mbed-os/v5.15/apis/serial.html#serial-hello-world
 */

#include <cstring>
#include "mbed.h"
#include "platform/mbed_thread.h"

#define DEMO_FOR_UART 1
#define DEMO_FOR_RTOS_THREAD 1
#define DEMO_FOR_WIFI_LED 1
#define WIFI_SCAN_SSID_PRNT 0

#ifdef DEMO_FOR_UART
Serial pc(USBTX, USBRX); // tx, rx
Serial device(MBED_CONF_APP_UART1_TX, MBED_CONF_APP_UART1_RX); // tx, rx
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
DigitalOut ledwifi(LED7, 1);

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

const char *wifi_get_ssid(WiFiAccessPoint *ap)
{
    return ap->get_ssid();
}

bool ssid_find(const char *ssid)
{
    WiFiAccessPoint *ap;
    bool ret = false;

    printf("WiFi Scan:\n");
    int count = wifi->scan(NULL,0);
    if (count <= 0) {
        printf("scan() failed with return value: %d\n", count);
        return ret;
    }

    /* Limit number of network arbitrary to 15 */
    count = count < 15 ? count : 15;
    ap = new WiFiAccessPoint[count];
    count = wifi->scan(ap, count);
    if (count <= 0) {
        printf("scan() failed with return value: %d\n", count);
        return ret;
    }
    printf("Total %d networks available. We need yo found %s SSID\n", count, ssid);
    for (int i = 0; i < count; i++) {
#if WIFI_SCAN_SSID_PRNT
        printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\n", ap[i].get_ssid(),
               sec2str(ap[i].get_security()), ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
               ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5], ap[i].get_rssi(), ap[i].get_channel());
#endif
#if 1
	if (!strcmp(ssid, wifi_get_ssid(&ap[i]))) {
            printf("ID: %d found SSID: %s.\n", i, wifi_get_ssid(&ap[i]));
            ret = true;
            break;
        }
#endif
    }
    delete[] ap;
    return ret;
}

bool connect_flag = false;
int wifi_connect(/* const char *ssid, const char *pwd */)
{
    //printf("\nConnecting to %s...\n", ssid);
    int ret = 0;
    bool stat = false;

    stat = ssid_find(MBED_CONF_APP_WIFI_SSID);
    if (!stat) {
       printf("Scan no found, so call wifi_disconnect");
    }

#if 1
    ret = wifi->connect(MBED_CONF_APP_WIFI_SSID,
		    MBED_CONF_APP_WIFI_PASSWORD,
		    NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error: %d\n", ret);
        return -1;
    }
#endif
    printf("Success\n\n");
    printf("MAC: %s\n", wifi->get_mac_address());
    printf("IP: %s\n", wifi->get_ip_address());
    printf("Netmask: %s\n", wifi->get_netmask());
    printf("Gateway: %s\n", wifi->get_gateway());
    printf("RSSI: %d\n\n", wifi->get_rssi());
    ledwifi.write(0);
    connect_flag = true;
    return 0;
}

void wifi_disconnect()
{
    ledwifi.write(1);
    wifi->disconnect();
    connect_flag = false;
}

void wifi_thread()
{
    bool running = true, stat = false;
    int ret = 0;

    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        printf("ERROR: No WiFiInterface found.\n");
        //running = false;
    }
#if 1
REDO:
    ret = wifi_connect(/* MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD */);
    if (ret != 0) {
        printf("\nConnection error: %d\n", ret);
        //running = false;
    }
#endif

    while (running) {
        stat = ssid_find(MBED_CONF_APP_WIFI_SSID);
        if (!stat) {
            printf("Scan no found, so call wifi_disconnect");
            wifi_disconnect();
        }
	if (stat && !connect_flag)
		goto REDO;
        thread_sleep_for(1000);
    }
    printf("\nwifi thread has Down\n");
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
        if (pc.readable()) {
            device.putc(pc.getc());
        }
        if (device.readable()) {
            pc.putc(device.getc());
        }
    }
#endif
}
