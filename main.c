#include <windows.h>
#include <stdio.h>
#include <time.h>

void configureSerialPort(HANDLE hSerial) {
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        printf("Error getting COM state\n");
        return;
    }

    dcbSerialParams.BaudRate = CBR_115200;  // Set baud rate
    dcbSerialParams.ByteSize = 8;         // Data bits (8)
    dcbSerialParams.StopBits = ONESTOPBIT; // Stop bits (1)
    dcbSerialParams.Parity = NOPARITY;    // Parity (None)

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        printf("Error setting COM state\n");
        return;
    }

    // Configure timeouts
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;          // Max time between two reads
    timeouts.ReadTotalTimeoutConstant = 50;     // Total read timeout
    timeouts.ReadTotalTimeoutMultiplier = 10;   // Multiplier for total read timeout
    timeouts.WriteTotalTimeoutConstant = 50;    // Total write timeout
    timeouts.WriteTotalTimeoutMultiplier = 10;  // Multiplier for total write timeout

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        printf("Error setting timeouts\n");
    }
}

void sendATCommand(HANDLE hSerial, const char *atCommand) {
    DWORD bytesWritten;
    printf(">> %s",atCommand);
    if (!WriteFile(hSerial, atCommand, (DWORD)strlen(atCommand), &bytesWritten, NULL)) {
        printf("Error writing to COM port\n");
        return;
    }
}

void recieveResponse(HANDLE hSerial) {
    char buffer[128];
    DWORD bytesRead;
    DWORD timeout = 1000;  // Timeout for ReadFile in milliseconds
    printf("<< \n");
    while (1) {
        // Set the timeout for ReadFile to prevent blocking forever
        COMMTIMEOUTS timeouts;
        GetCommTimeouts(hSerial, &timeouts);  // Get the current timeouts
        timeouts.ReadTotalTimeoutConstant = timeout;  // Set a read timeout
        SetCommTimeouts(hSerial, &timeouts);  // Apply the timeout settings

        if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';  // Null-terminate the buffer
                printf("%s", buffer);
            } else {
                // If no data was read, break the loop after a period of inactivity
                printf("No more data received\n");
                break;
            }
        } else {
            printf("Error reading from COM port\n");
            break;  // Exit on error
        }
    }
}

void wait(int milliseconds) {
    clock_t start_time = clock(); // Get the current time
    while (clock() < start_time + milliseconds * CLOCKS_PER_SEC / 1000);// Loop until the time has passed
}

int main() {
    // Open the COM port

    const char* commandList[] = {"AT+REBOOT\r\n"};
    size_t num_elements = sizeof(commandList) / sizeof(commandList[0]);

    HANDLE hSerial = CreateFile("COM9", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error opening COM port\n");
        return 1;
    }

    // Configure the serial port
    configureSerialPort(hSerial);

    for (int i = 0; i < num_elements; i++) {
        sendATCommand(hSerial,commandList[i]);
        recieveResponse(hSerial);
    }

    // Close the COM port after communication
    CloseHandle(hSerial);

    return 0;
}
