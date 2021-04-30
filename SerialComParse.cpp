#define _CRT_SECURE_NO_WARNINGS 1

#include <iostream>
#include <Windows.h>
#include <string>
#include <fstream>

#include "InputParser.h"


HANDLE setupHandle(std::string com_path, int baud_rate = 57600, int byte_size = 8)
{
    HANDLE serialHandle;

    serialHandle = CreateFile(com_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    // Do some basic settings
    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);

    GetCommState(serialHandle, &serialParams);
    serialParams.BaudRate = baud_rate;
    serialParams.ByteSize = byte_size;
    serialParams.StopBits = 1;
    serialParams.Parity = NOPARITY;
    SetCommState(serialHandle, &serialParams);

    // Set timeouts
    COMMTIMEOUTS timeout = { 0 };
    timeout.ReadIntervalTimeout = 50;
    timeout.ReadTotalTimeoutConstant = 50;
    timeout.ReadTotalTimeoutMultiplier = 50;
    timeout.WriteTotalTimeoutConstant = 50;
    timeout.WriteTotalTimeoutMultiplier = 10;

    SetCommTimeouts(serialHandle, &timeout);

   
    return serialHandle;
}

std::string getBinName()
{
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char timeBuf[15];
    strftime(timeBuf, 15, "%y%m%d_%H%M", timeinfo);

    std::string binName(timeBuf);
    binName += ".bin";
    return binName;
}

void startInfo() {
    time_t now = time(0);
    std::cout << "Program starts: " << ctime(&now) << std::endl;
}

int main(int argc, char** argv)
{
    InputParser input(argc, argv);
    std::string com_path;

    // HELP:
    if (input.cmdOptionExists("-h")) {
        std::cout << "Help for SerialComParse program. Set the path for file with commends for sending via serial port.\n\nOptions:\n";
        std::cout << "\t-f    Specify .txt file path with commands (Mandatory)\n";
        std::cout << "\t-p    Specify Port name (Mandatory)\n";
        std::cout << "\t-b    Set Baudrate (Default = 57600)\n";
        std::cout << "\t-st   Set Stop Bits (Default = 1)\n";
        std::cout << "\t-bs   Byte size (Default = 8)\n" << std::endl;

        return 0;
    }

    //Mandatory Options:
    if (!input.cmdOptionExists("-f")) {
        std::cout << "InputError: No file path specified! '-f' option is necessery Insert comand file path" << std::endl;
        return 1;
    }
    const std::string& filename = input.getCmdOption("-f");
    if (filename.empty()) {
        std::cout << "InputError: No file path has been s pecified! Insert comand file path" << std::endl;
        return 1;
    }

    if (!input.cmdOptionExists("-p")) {
        std::cout << "InputError: No COM port has been specified" << std::endl;
        return 1;
    }
    const std::string& com = input.getCmdOption("-p");
    if (filename.empty()) {
        std::cout << "InputError: No COM port has been specified" << std::endl;
        return 1;
    }
    else {
        com_path = "\\\\.\\" + com;
    }

    //Optional Parameters:
    int baudRate = 57600;
    int byteSize = 8;
    int stopBits = 1;
    std::string parity = "NOPARITY";
    
    if (input.cmdOptionExists("-b")){
        baudRate = std::stoi(input.getCmdOption("-b"));
    }

    if (input.cmdOptionExists("-st")) {
        stopBits = std::stoi(input.getCmdOption("-st"));
    }

    if (input.cmdOptionExists("-bs")) {
        byteSize = std::stoi(input.getCmdOption("-bs"));
    }

    HANDLE Serial = setupHandle(com_path, baudRate, stopBits);

    if (Serial == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            std::cout << "ConnectError: Serial Port does not exist!" << std::endl;
        }
        std::cout << "ConnectError: No connection" << std::endl;
        return 2;
    }

    std::string binFilePath = getBinName();
    
    std::string cmd_string;
    std::ifstream cmd_file(filename);
    std::ofstream bin_file(binFilePath, std::ios::out | std::ios::binary);

    if (!bin_file){
        std::cout << "ERROR: No binary File" << std::endl;
        return 2;
    }



    if (cmd_file.is_open()){
        startInfo();
        while (std::getline(cmd_file, cmd_string)){
            char msgBuf[256] = { 0 };
            //std::string msg_string;
            
            DWORD dwBytesRead = 0;

            WriteFile(Serial, cmd_string.c_str(), cmd_string.size(), &dwBytesRead, NULL);
            if (ReadFile(Serial, msgBuf, 256, &dwBytesRead, NULL)) {
                std::string msg_string(msgBuf);
                //msg_string.shrink_to_fit();
                bin_file.write(msg_string.c_str(), msg_string.size());
                //bin_file.write((char*)&msgBuf, sizeof(msgBuf));
                bin_file << "\r\n";
            }
            else {
                std::cout << "No msg read" << std::endl;
            }
        }
    }
    else {
        std::cout << "Can not open the file" << std::endl;
    }

    
    bin_file.close();
    cmd_file.close();
    CloseHandle(Serial);
    return 0;
}
