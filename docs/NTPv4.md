## information

The following docuemnt summarizes section 1-6 in RFC 5905, NTPv4 Specification [page 3-15].

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
1. A timescale: is a continuous and monotonically increasing time scale. Meaning that time is always moving forward and never backward.
2. Coordinated Universal Time  (UTC): It represents mean solar time as disseminated by national standards laboratories.
3. Mean solar time: is the time that is based on the mean solar day, which is the average length of a solar day over a year.
4. `Timestamp`, T(t):
   - Represents either UTC date or time offset from UTC at running time t.
  
5. `Frequency offset`, R(t):
   - The offset, or how much the local time differs from the UTC per second.
   - Defined in the unit s/s, or more usefully, ppm (10e-6s/s.)
6. `Aging rate`, D(t):
   - The rate of the the absolute time offset from UTC grows.
   - The first derivative of the frequency offset with respect to time (dR/dt).
7. T(t) = T(t_0) + R(t_0)(t-t_0) + 1/2 * D(t_0)(t-t_0)^2 + e
   (e is an error term)

## 5. Implementation Model
1. Implementation model: Architecture of how NTP's can be implemented, see chapter for visua lrepresentation.
2. Example implementation: Example shown is a typical, multithreaded implementation. Each peer in the system sends and receive packets from a remote server while also sending in a directional pattern to other peers. The system process selection and cluster algorithms for the system process before doing a combine algorithm. To better find the "truechimers" and not the "falsetickers", a loop filter is used in the clock disciple process. This is later sent to the VFO that is part of the clock-adjust process. In this process, the time is changed and the offset frequency managed with previous and current data/information. The frequency of clients sending to servers is done by the time between messages to be 2^tau seconds. Client/servers have the server answer immediate but with client to client must their tau intervals fit each other using the NTP protocl.
3. `Truechimers`  - A clock system that fits a previously done standard for time accuracy and timekeeping.
4. `Falsetickers` - A clock system that does not hold time accuracy or shows inconcistent time.
5. `VFO` - Stands for variable frequency oscillator, the part of the process that controls the time and frequency of system clock.
6. `tau` - A number each client sets to determine the polling interval for NTP protocols. The value of tau is needed to be common between clients in client to client implementations to make reqeusting and sending packets consistent.

## 6. Data Types
1. All NTP time values are represented in twos-complement format, with bits numbered in big-endian. 
2. There are three NTP time formats:
   - a 128-bit date format,
   - a 64-bit timestamp format and
   - a 32-bit short format.
3. The 128-bit date format is used to represent the current date and time.
4. The 64-bit timestamp format is used in packet headers and other places with limited word size.
5. The 32-bit short format is used in delay and dispersion header fields where the full resolution and range of the other formats are not justified.
6. Time is started from a point in time and the two's complement used to count positively forward in time and negative for back in time.
7. To determine the integer era and timestamp given s,

   - era = s / 2^(32) and timestamp = s - era * 2^(32), it works for positive and negative time.

   To determine s with given timestamp and era.

   - s = era * 2^(32) + timestamp
   
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

- Q: **Why does is a common tau needed for clinet to client implementations?**
- A: As client to server has the server respond with the time information / packet immediately. Client to client however is not responding immediately as they often send with their own tau number. This makes it important to have an implementation of making the tau value be common between clients, especially as this dynamically changes.