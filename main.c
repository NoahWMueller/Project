#include <windows.h>
#include <stdio.h>
#include <conio.h>

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
                printf("<<\n");
                break;
            }
        } else {
            printf("Error reading from COM port\n");
            break;  // Exit on error
        }
    }
}

void sendATCommand(HANDLE hSerial, const char *atCommand) {
    DWORD bytesWritten;
    printf(">> %s",atCommand);
    if (!WriteFile(hSerial, atCommand, (DWORD)strlen(atCommand), &bytesWritten, NULL)) {
        printf("Error writing to COM port\n");
        return;
    }
    recieveResponse(hSerial);
}


int main() {
    // Open the COM port
    HANDLE hSerial = CreateFile("COM9", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error opening COM port\n");
        return 1;
    }

    // Configure the serial port
    configureSerialPort(hSerial);

    while(1){
        if (_kbhit()){
            char ch = _getch();
            if (ch == 27) {
                break;
            } else if (ch == 'q' || ch == 'Q') {                     // reboot/reconnect
                sendATCommand(hSerial,"AT+REBOOT\r\n");
            } else if (ch == 'w' || ch == 'W') {                     // disconnect
                sendATCommand(hSerial,"AT+DSCA\r\n");
            } else if (ch == 'e' || ch == 'E') {                     // undiscoverable
                sendATCommand(hSerial,"AT+PAIR=0\r\n");
            } else if (ch == 'r' || ch == 'R') {                     // discoverable
                sendATCommand(hSerial,"AT+PAIR=1\r\n");
            } else if (ch == 't' || ch == 'T') {                     // clear paired list
                sendATCommand(hSerial,"AT+PLIST=0\r\n");
            } else if (ch == 'y' || ch == 'Y') {                     // answer call
                sendATCommand(hSerial,"AT+HFPANSW\r\n");
            } else if (ch == 'u' || ch == 'U') {                     // decline call
                sendATCommand(hSerial,"AT+HFPCHUP\r\n");
            }
        }
    }

    // Close the COM port after communication
    CloseHandle(hSerial);

    return 0;
}
