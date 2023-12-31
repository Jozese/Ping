//TODO: Handle ICMP error info
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")


int main(int argc, char** argv)
{
    bool persistent = false;
    int amount = 0;
    int delay = 1000;
    int timeout = 10000;
    int count = 5;

    if (argc == 1) {
        std::cout << "Usage: " + std::string(argv[0]) + " [-p] [-d delay] [-t timeout]\n\t[-c count] [-a amount] target_name\n" << std::endl;
        std::cout << "Options: \n\t-p\tKeeps sending ICMP echo requests until stopped.\n\t-d\tDelay in milliseconds between each ICMP echo request.\n\t-t\tMaximum time to wait for an ICMP echo reply in milliseconds. Default 10000ms if not specified.\n\t-c\tAmount of ICMP echo requests to send per IP. Ignored if persistent. Default 5 requests.\n\t-a\tAmount of IPs to check per domain name. Not specified or 0 will check all IPv4s of the target name.";
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-p") {
            persistent = true;
        }
        else if (std::string(argv[i]) == "-a") {
            amount = std::stoi(argv[i + 1]);
        }
        else if (std::string(argv[i]) == "-d")
        {
            delay = std::stoi(argv[i + 1]);
        }
        else if (std::string(argv[i]) == "-t")
        {
            timeout = std::stoi(argv[i + 1]);
        }
        else if (std::string(argv[i]) == "-c")
        {
            count = std::stoi(argv[i + 1]);
        }
    }


    //Ping packet payload
    const uint16_t PAYLOAD_SIZE = 1;
    
    //Echo reply size
    const uint32_t REPLY_SIZE = sizeof(ICMP_ECHO_REPLY) + PAYLOAD_SIZE + 8;


	std::string domainName = argv[argc-1];

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

	HANDLE icmpCtx = IcmpCreateFile();
	if (icmpCtx == INVALID_HANDLE_VALUE) {
		std::cout << "Could not create ICMP context " << std::endl;
	}

	struct addrinfo* result = nullptr;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


    //Resolving domain name 

	int res = getaddrinfo(domainName.c_str(), nullptr, &hints, &result);
    std::vector<std::string> ips;

    //Only getting Ipv4s
    if (res == 0) {
        for (struct addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
            char ipString[INET_ADDRSTRLEN];

            if (ptr->ai_family == AF_INET) {
                struct sockaddr_in* ipv4 = (struct sockaddr_in*)ptr->ai_addr;
                inet_ntop(AF_INET, &(ipv4->sin_addr), ipString, INET_ADDRSTRLEN);
            }
            
            ips.emplace_back(std::string(ipString));
        }

        freeaddrinfo(result);
    }
    else
    {
        std::cout << "Could not resolve domain" << std::endl;
        exit(EXIT_FAILURE);
    }
    

    if (amount == 0) {
        amount = ips.size();
    }

    while (true)
    {
        for (int k = 0; k < count; k++) {
            for (int i = 0; i < amount; i++) {
                IN_ADDR ipDest{};
                if (InetPtonA(AF_INET, ips[i].c_str(), &ipDest) != 1) {
                    std::cout << "Could not parse " << i << std::endl;
                    continue;
                }

                unsigned char requestBuf[PAYLOAD_SIZE];
                unsigned char replyBuf[REPLY_SIZE];

                //https://learn.microsoft.com/en-us/windows/win32/api/icmpapi/nf-icmpapi-icmpsendecho
                uint32_t reply = IcmpSendEcho(icmpCtx, ipDest.S_un.S_addr,
                    requestBuf, PAYLOAD_SIZE, NULL, replyBuf, REPLY_SIZE, timeout);

                const ICMP_ECHO_REPLY* r = (const ICMP_ECHO_REPLY*)replyBuf;

                struct in_addr addr;
                addr.s_addr = r->Address;

                std::cout << "Reply from " << inet_ntoa(addr) << " | Data size: " << r->DataSize * 8 << " bits | Time: " << r->RoundTripTime << "ms | TTL: " << (uint16_t)r->Options.Ttl << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            }
        }
        if (!persistent) {
            break;
        }
    }
    

    IcmpCloseHandle(icmpCtx);
    return EXIT_SUCCESS;
}
