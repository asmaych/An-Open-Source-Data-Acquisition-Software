#include <stdio.h>
#include <libserialport.h>

int main() {
    struct sp_port **ports;
    int result = sp_list_ports(&ports);

    if (result != SP_OK) {
        printf("Failed to list ports\n");
        return 1;
    }

    printf("Available serial ports:\n");
    for (int i = 0; ports[i]; i++) {
        printf("  %s\n", sp_get_port_name(ports[i]));
    }

    // Example: open the first port
    struct sp_port *port = ports[0];
    sp_open(port, SP_MODE_READ);

    // Set baud rate
    sp_set_baudrate(port, 9600);

    // Read data (non-blocking)
    char buf[100];
    int bytes_read = sp_nonblocking_read(port, buf, sizeof(buf));

    if (bytes_read > 0) {
        buf[bytes_read] = '\0';
        printf("Received: %s\n", buf);
    } else {
        printf("No data received.\n");
    }

    sp_close(port);
    sp_free_port_list(ports);

    return 0;
}
