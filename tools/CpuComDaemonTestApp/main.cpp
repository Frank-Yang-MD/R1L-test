/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "TestNative.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

namespace cpucom = com::mitsubishielectric::ahu::cpucom;

enum MenuID {
    Exit,
    Subscribe,
    Unsubscribe,
    Send,
    File,
    TestFarmStart,
    TestFarmStop,
    CmndLineMod,
    Idle,
    defaultAnsw
};

int parse_options(int argc, char* argv[]);
void show_help(void);

int main(int argc, char* argv[])
{
    parse_options(argc, argv);
    cpucom::CpuComDaemonClient cpuComDaemonClient;

    unsigned int answer = Idle;
    std::string temp;

    bool stopFlag = false;
    // clang-format off
    std::cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~TestCpuCommunication~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    // clang-format on

    while (!stopFlag) {
        switch (answer) {
        case Subscribe:
            cpuComDaemonClient.subscribe();
            answer = Idle;
            break;
        case Unsubscribe:
            cpuComDaemonClient.unsubscribe();
            answer = Idle;
            break;
        case Send:
            cpuComDaemonClient.send();
            answer = Idle;
            break;
        case File:
            cpuComDaemonClient.file();
            answer = Idle;
            break;
        case TestFarmStart:
            cpuComDaemonClient.testFarmStart();
            answer = Idle;
            break;
        case TestFarmStop:
            cpuComDaemonClient.testFarmStop();
            answer = Idle;
            break;
        case CmndLineMod:
            cpuComDaemonClient.cmndLineMod();
            answer = Idle;
            break;
        case Exit:
            stopFlag = true;
            break;
        case Idle:
            std::cout << "  Choice needed:\n\n";
            std::cout << "      subscribe ........... -> 1\n";
            std::cout << "      unsubscribe ......... -> 2\n";
            std::cout << "      send data ........... -> 3\n";
            std::cout << "      Read from file ...... -> 4\n";
            std::cout << "      Test Farm Start ..... -> 5\n";
            std::cout << "      Test Farm Stop ...... -> 6\n\n";
            std::cout << "      Run in cmndLine mod   -> 7\n\n";
            std::cout << "  Exit .................... -> 0\n";
            std::getline(std::cin, temp);
            if (!temp.empty()) {
                if (std::all_of(temp.begin(), temp.end(), ::isdigit)) {
                    answer = std::stoi(temp, nullptr, 10);
                }
                else {
                    answer = defaultAnsw;
                }
            }
            break;
        default:
            std::cout << "***unknown command***\n";
            answer = Idle;
            break;
        }
    }
    return 0;
}

int parse_options(int argc, char* argv[])
{
    int c;
    while ((c = getopt(argc, argv, "h")) != -1) {
        switch (c) {
        case 'h':
            show_help();
            exit(0);
        }
    }
    return 0;
}

void show_help()
{
    std::cout << "  CmndLine mod examples:\n";
    std::cout << "     subscribe 11,80\n";
    std::cout << "     unsubscribe 11,80\n";
    std::cout << "     subscribelist 11,80 1f,1c 3e,a1\n";
    std::cout << "     send 1b,3f 23,55,2c\n";
    std::cout << "     exit\n";
}