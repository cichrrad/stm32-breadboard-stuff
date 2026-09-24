#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

// Struct to hold your parsed data - ready for open62541 mapping
typedef struct {
    unsigned long time_ms;
    float temperature;
    float pressure;
    float humidity;
} SensorData;

// Function to configure the serial port
int setup_serial_port(const char *portname) {
    int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        fprintf(stderr, "Error %d opening %s: %s\n", errno, portname, strerror(errno));
        return -1;
    }

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "Error %d from tcgetattr: %s\n", errno, strerror(errno));
        close(fd);
        return -1;
    }

    // Set Baud Rate to 115200
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    // 8-bit chars, no parity, 1 stop bit (8N1)
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     
    tty.c_cflag &= ~(PARENB | PARODD);              
    tty.c_cflag &= ~CSTOPB;                         
    
    // Ignore modem controls, enable reading
    tty.c_cflag |= (CLOCAL | CREAD);                
    // Disable hardware flow control
    tty.c_cflag &= ~CRTSCTS;

    // Enable Canonical Mode (reads line by line instead of raw bytes)
    tty.c_lflag |= ICANON;
    // Disable echo, signaling chars, etc.
    tty.c_lflag &= ~(ECHO | ECHOE | ECHONL | ISIG);

    // Disable software flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    // Do not alter incoming bytes
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

    // Raw output
    tty.c_oflag &= ~OPOST;

    // Save termios settings
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Error %d from tcsetattr: %s\n", errno, strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

int main() {
    const char *portname = "/dev/ttyACM0";
    int serial_fd = setup_serial_port(portname);
    
    if (serial_fd < 0) {
        return EXIT_FAILURE;
    }

    printf("Listening on %s at 115200 baud...\n", portname);

    SensorData current_data = {0};
    char read_buf[256];
    
    while (1) {
        // Because of canonical mode, read() waits until it sees a '\n'
        int n = read(serial_fd, read_buf, sizeof(read_buf) - 1);
        
        if (n > 0) {
            read_buf[n] = '\0'; // Null-terminate the string

            // Parse the incoming line using sscanf
            if (strncmp(read_buf, "Time:", 5) == 0) {
                sscanf(read_buf, "Time: %lu ms", &current_data.time_ms);
            } 
            else if (strncmp(read_buf, "Temp:", 5) == 0) {
                sscanf(read_buf, "Temp: %f C", &current_data.temperature);
            } 
            else if (strncmp(read_buf, "Press:", 6) == 0) {
                sscanf(read_buf, "Press: %f hPa", &current_data.pressure);
            } 
            else if (strncmp(read_buf, "Hum:", 4) == 0) {
                sscanf(read_buf, "Hum: %f %%", &current_data.humidity);
                
                printf("Updated Struct -> Time: %lu, Temp: %.2f, Press: %.2f, Hum: %.2f\n", 
                        current_data.time_ms, 
                        current_data.temperature, 
                        current_data.pressure, 
                        current_data.humidity);
            }
        } else if (n < 0) {
            fprintf(stderr, "Error reading: %s\n", strerror(errno));
            break;
        }
    }

    close(serial_fd);
    return EXIT_SUCCESS;
}