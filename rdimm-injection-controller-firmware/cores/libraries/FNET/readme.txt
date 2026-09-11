Teensy FNET TCP/IP Stack.
==================
Can be used in conjunction with https://github.com/vjmuzik/TeensyASIXEthernet
Example project uses TeensyASIXEthernet driver.

The stack provides following protocols and services:
- Supported Platforms:
	-Arduino
    -Teensy
- Bare-metal TCP/IP stack. No underlying RTOS is required, although it can be used with it.
- Certified logos for:
	- Golden IPv6 Ready.
	- Microsoft Azure IoT.
	- Apple Bonjour. 
- Non-blocking Socket API (DGRAM, STREAM, RAW).
- Core Protocols:
	- TCP (Transmission Control Protocol).
	- UDP (User Datagram Protocol).
	- IPv4.
		- ICMPv4 (Internet Control Message Protocol).
		- IGMP (Internet Group Management Protocol).
		- ARP (Address Resolution Protocol).
	- IPv6. 
		- "IPv6 Ready Logo Certified". Passed IPv6 Core Protocols Conformance and Interoperability tests.
		- ICMPv6 (Internet Control Message Protocol).
		- MLDv1 (Multicast Listener Discovery).
		- Neighbor Discovery for IPv6.
		- IPv6 Stateless Address Autoconfiguration.
		- Path MTU Discovery for IPv6
- Network Interfaces:
	- Non-specific
- Services:
	- HTTP server:
		- HTTP/1.0 or HTTP/0.9 protocols.
		- GET and POST requests.
		- CGI and SSI.
		- Basic access authentication.
		- HTTP over TLS (HTTPS).
	- TELNET server. 
	- DHCPv4 client and server.
	- Auto-IP service. Passed "Link-Local Address Allocation", Bonjour Conformance Test.
	- Azure IoT Hub client adapter. Microsoft Azure certified.
	- DNS client/resolver.
	- Link-Detection service.
	- Multicast DNS (mDNS) "Bonjour" Server/Responder. Passed the Bonjour Conformance Test.
	- LLMNR Server/Responder.
	- PING service.
	- SNTP client.	
	- TFTP server and client. Both Read and Write requests are supported.
	- TLS using mbedTLS library.
	- ROM File System.
	- On-chip Flash Memory driver.
	- Command line shell.

FNET project directory structure:
	fnet                        - FNET root directory.
	|-- src                - FNET TCP/IP stack.
	|    |-- stack                  - FNET TCP/IP stack platform-independent source code.
	|    |-- service                - FNET Services.
	|    |    |-- autoip              - Auto-IP service.
	|    |    |-- azure               - Azure IoT Hub adapter.
	|    |    |-- bench               - Benchmark client and server.
	|    |    |-- dhcp                - DHCPv4 client and server.
	|    |    |-- dns                 - DNS client/resolver.
	|    |    |-- flash               - Flash Memory driver.
	|    |    |-- fs                  - File System driver.
	|    |    |-- http                - HTTP Server service.
	|    |    |-- link                - Link-Detection service.
	|    |    |-- llmnr               - LLMNR server/responder.
	|    |    |-- mdns                - mDNS server/responder.
	|    |    |-- ping                - PING service.
	|    |    |-- poll                - Polling Mechanism library.
	|    |    |-- serial              - Serial Input/Output driver.
	|    |    |-- shell               - Command Shell service.
	|    |    |-- sntp                - SNTP client.
	|    |    |-- telnet              - TELNET server.
	|    |    |-- tftp                - TFTP server and client.
	|    |    |-- tls                 - TLS library.
	|    |-- port                   - FNET port-specific source code.
	|         |-- compiler            - Compiler-specific source code.
