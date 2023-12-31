# ICMP Ping Utility

 ICMP Ping command-line tool built in C++ for sending ICMP echo requests to specified domains or IP addresses.

## Overview


- Send ICMP echo requests to specified domains/IPs.
- Set various options like timeout, delay, and the number of requests.
- Check multiple IPv4 addresses associated with a domain.

## Prerequisites

- **Windows OS**: This utility is designed for Windows and utilizes WinSock2 and related libraries.
- **C++ Compiler**: You'll need a C++ compiler compatible with the used libraries.

## Usage

```bash
Usage: ping [-p] [-d delay] [-t timeout] [-c count] [-a amount] target_name

Options:
  -p   Keeps sending ICMP echo requests until stopped.
  -d   Delay in milliseconds between each ICMP echo request.
  -t   Maximum time to wait for an ICMP echo reply in milliseconds. Default 10000ms if not specified.
  -c   Amount of ICMP echo requests to send per IP. Ignored if persistent. Default 5 requests.
  -a   Amount of IPs to check per domain name. Not specified or 0 will check all IPv4s of the target name.
```

Replace \`target_name\` with the domain name or IP address you want to ping.

## Example Usage

- Ping a domain continuously with a delay of 500ms:
  ```bash
  ping -p -d 500 example.com
  ```

- Send 10 ICMP echo requests with a timeout of 5000ms:
  ```bash
  ping -c 10 -t 5000 8.8.8.8
  ```
- Send 10 ICMP echo requests with a timeout of 5000ms to the first IP address dns query returns:
  ```bash
  ping -c 10 -t 5000 -a 1 microsoft.com
  ```

