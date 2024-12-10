#include <windows.h>
#include <stdio.h>

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
    if (!WriteFile(hSerial, atCommand, (DWORD)strlen(atCommand), &bytesWritten, NULL)) {
        printf("Error writing to COM port\n");
        return;
    }

        // Read response from the module
    char buffer[128];
    DWORD bytesRead;
    if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        buffer[bytesRead] = '\0';  // Null-terminate the buffer
        printf("Response: %s\n", buffer);
    } else {
        printf("Error reading from COM port\n");
    }
}


int main() {
    // Open the COM port
    HANDLE hSerial = CreateFile("COM8", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error opening COM port\n");
        return 1;
    }

    // Configure the serial port
    configureSerialPort(hSerial);

    // Infinite loop to continuously read terminal input and send as AT command
    char userInput[256];
    while (1) {
        printf("Enter AT command (or 'exit' to quit): ");
        if (fgets(userInput, sizeof(userInput), stdin) == NULL) {
            printf("Error reading input\n");
            break;
        }

        // Remove trailing newline character from input
        userInput[strcspn(userInput, "\n")] = '\0';

        // If the user types "exit", break the loop and quit
        if (strcmp(userInput, "exit") == 0) {
            break;
        }

        // Append \r\n to the userInput using strcat_s
        strcat_s(userInput, sizeof(userInput), "\r\n");

        // Send the AT command and display the response
        sendATCommand(hSerial, (userInput));

    }

    // Close the COM port after communication
    CloseHandle(hSerial);

    return 0;
}
