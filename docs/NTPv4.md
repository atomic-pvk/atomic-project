## information

The following docuemnt summarizes section 1-6 in RFC 5905, NTPv4 Specification [page 3-15]. The document is linked as a bookmark in our Slack channel `chupa-chups`.

link to the [RFC 5905](https://datatracker.ietf.org/doc/html/rfc5905#ref-ref9)
link to the [RFC 2119](https://datatracker.ietf.org/doc/html/rfc2119)

## 1. Introduction

1. NTPv4 is used to synchronize system clocks among a set of distributed time servers and clients.
2. NTPv4 is an update to NTPv3, and is designed to be backward compatible with NTPv3.
3. Requirement notation, terminology, and conventions are defined in [RFC 2119](https://datatracker.ietf.org/doc/html/rfc2119).

- `MUST`
- `MUST NOT`
- `REQUIRED`
- `SHALL`
- `SHALL NOT`
- `SHOULD`
- `SHOULD NOT`
- `RECOMMENDED`
- `MAY`
- `OPTIONAL`

## 2. Modes of Operation

1. An NTP implementation operates as a primary server, secondary server, or client.
2. A primary server is a time server that is directly connected to a reference clock.
3. A secondary server is a time server that is synchronized to a primary server.
4. A client is a system that synchronizes its clock to a server.

## 3. Protocol Modes

1. There are three NTP protocol variants: symmetric, client/server, and broadcast.
2. The symmetric mode is used for server-to-server communication.
3. The client/server mode is used for client-to-server communication.
4. The broadcast mode is used for server-to-client communication.
5. There are different association and packet modes.
6. Dynamic server discovery. There are two special associations:

   - manycast client and
   - manycast server, which provide a dynamic server discovery function.

7. There are two types of manycast client associations:

   - persistent and
   - ephemeral (ephermal means short-lived).

8. The persistent manycast client sends client (mode 3) packets to a designated IPv4 or IPv6 broadcast or multicast group address.

## 4. Definitions

## 5. Implementation Model

## 6. Data Types

### Recap of the RFC 2119 section 1-6

- Q: **What does NTP stand for?**
- A: Network Time Protocol

- Q: **What is the purpose of NTP?**
- A: NTP is used to synchronize system clocks among a set of distributed time servers and clients.

- Q: **Why is it necessary to synchronize system clocks?**
- A: It is necessary to synchronize system clocks to ensure that the time is consistent across all systems.

- Q: **What would be the impact of unsynchronized system clocks?**
- A: The impact of unsynchronized system clocks is that it would be difficult to correlate events across systems.

- Q: **What is the difference between NTPv3 and NTPv4?**
- A: NTPv4 is an update to NTPv3, and is designed to be backward compatible with NTPv3.

- Q: **What is a server?**
- A: A server is a time server that is directly connected to a reference clock.

- Q: **What is a client?**
- A: A client is a system that synchronizes its clock to a server.

- Q: **What are association and packets mode in NTP?**
- A: An association is a relationship between a client and a server. A packet mode is a mode of operation for NTP.

- Q: **What could be a mode of operation for NTP?**
- A: An NTP implementation operates as a primary server, secondary server, or client.

- Q: **What is the purpose of the manycast client and manycast server?**
- A: The manycast client and manycast server provide a dynamic server discovery function.

- Q: **What is the difference between a persistent and ephemeral manycast client association?**
- A: The persistent manycast client sends client (mode 3) packets to a designated IPv4 or IPv6 broadcast or multicast group address. The ephemeral manycast client is short-lived.

- Q: **What is IPv4 and IPv6?**
- A: IPv4 and IPv6 are versions of the Internet Protocol. Internet Protocol version 4 (IPv4) is the fourth version of the Internet Protocol. Internet Protocol version 6 (IPv6) is the most recent version of the Internet Protocol. The main difference between IPv4 and IPv6 is the number of IP addresses they can support.
