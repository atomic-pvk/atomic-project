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
2. Coordinated Universal Time (UTC): It represents mean solar time as disseminated by national standards laboratories.
3. Mean solar time: is the time that is based on the mean solar day, which is the average length of a solar day over a year.
4. `Timestamp`, T(t):

   - Represents either UTC date or time offset from UTC at running time t.

5. `Frequency offset`, R(t):
   - The offset, or how much the local time differs from the UTC per second.
   - Defined in the unit s/s, or more usefully, ppm (10e-6s/s.)
6. `Aging rate`, D(t):
   - The rate of the the absolute time offset from UTC grows.
   - The first derivative of the frequency offset with respect to time (dR/dt).
7. T(t) = T(t_0) + R(t_0)(t-t_0) + 1/2 \* D(t_0)(t-t_0)^2 + e
   (e is an error term)

## 5. Implementation Model

1. Implementation model: Architecture of how NTP's can be implemented, see chapter for visua lrepresentation.
2. Example implementation: Example shown is a typical, multithreaded implementation. Each peer in the system sends and receive packets from a remote server while also sending in a directional pattern to other peers. The system process selection and cluster algorithms for the system process before doing a combine algorithm. To better find the "truechimers" and not the "falsetickers", a loop filter is used in the clock disciple process. This is later sent to the VFO that is part of the clock-adjust process. In this process, the time is changed and the offset frequency managed with previous and current data/information. The frequency of clients sending to servers is done by the time between messages to be 2^tau seconds. Client/servers have the server answer immediate but with client to client must their tau intervals fit each other using the NTP protocl.
3. `Truechimers` - A clock system that fits a previously done standard for time accuracy and timekeeping.
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

   - era = s / 2^(32) and timestamp = s - era \* 2^(32), it works for positive and negative time.

   To determine s with given timestamp and era.

   - s = era \* 2^(32) + timestamp

### 7. Data Structures

1. State machines is a model used to represent the different states that a system can be in, and the transition between those states in response to events.
2. State variables are variables that hold information about the current state of a system, for example reference identifier and offset. State variables are separated into classes according to their function in:

- Packet headers: represents the NTP header values in transmitted and recieved packets.
- Peer and poll processes: represents the contents of the association for each server separately.
- The system process: represents the state of the server as seen by its dependent clients
- Clock discipline process: represents the internal workings of the clock discipline algorithm.

3. The NTP packet header consists of an integral number of 32-bit words in network byte order. The packet format consists of three components: the header itself, one or more optional extension fields (used by the Autokey public key cryptographic algorithms) and an optional message authentication code, MAC, (used by both Autokey and the symmetric key cryptographic algorithm).
   The NTP packet is a UDP datagram where some fields use multiple words and others are packed in smaller fields within one word.
4. The most important state variables are:

<table>
  <thead>
    <tr>
      <th>Name</th>
      <th>Formula</th>
      <th>Description</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>leap</td>
      <td>leap</td>
      <td>leap indicator (LI)</td>
    </tr>
    <tr>
      <td>version</td>
      <td>version</td>
      <td>version number (VN)</td>
    </tr>
    <tr>
      <td>mode</td>
      <td>mode</td>
      <td>mode</td>
    </tr>
    <tr>
      <td>stratum</td>
      <td>stratum</td>
      <td>stratum</td>
    </tr>
    <tr>
      <td>poll</td>
      <td>poll</td>
      <td>poll exponent</td>
    </tr>
    <tr>
      <td>precision</td>
      <td>rho</td>
      <td>precision exponent</td>
    </tr>
    <tr>
      <td>rootdelay</td>
      <td>delta_r</td>
      <td>root delay</td>
    </tr>
    <tr>
      <td>rootdisp</td>
      <td>epsilon_r</td>
      <td>root dispersion</td>
    </tr>
    <tr>
      <td>refid</td>
      <td>refid</td>
      <td>reference ID</td>
    </tr>
    <tr>
      <td>reftime</td>
      <td>reftime</td>
      <td>reference timestamp</td>
    </tr>
    <tr>
      <td>org</td>
      <td>T1</td>
      <td>origin timestamp</td>
    </tr>
    <tr>
      <td>rec</td>
      <td>T2</td>
      <td>receive timestamp</td>
    </tr>
    <tr>
      <td>xmt</td>
      <td>T3</td>
      <td>transmit timestamp</td>
    </tr>
    <tr>
      <td>dst</td>
      <td>T4</td>
      <td>destination timestamp</td>
    </tr>
    <tr>
      <td>keyid</td>
      <td>keyid</td>
      <td>key ID</td>
    </tr>
    <tr>
      <td>dgst</td>
      <td>dgst</td>
      <td>message digest</td>
    </tr>
  </tbody>
</table>

Explaination of the fields and the associated packet variables:

- Leap Indicator, LI, (leap): 2-bit integer warning of an impending leap second to be inserted or deleted in the last minute of the current month (no warning, last minute has 61 seconds, last minute has 59 seconds or unknown).
- Version Number, VN, (version): 3-bit integer representing the NTP version number.
- Mode (mode): 3-bit integer representing the mode (reserved, client, server etc).
- Stratum (stratum): 8-bit integer representing the stratum (primary server, secondary server etc).
- Poll (poll): 8-bit signed integer representing the maximum interval between successive messages, in log2 seconds.
- Precision (rho): 8-bit signed integer representing the precision of the system clock, in log2 seconds.
- Root Delay (rootdelay): Total round-trip delay to the reference clock, in NTP short format.
- Root Dispersion (rootdisp): Total dispersion to the reference clock, in NTP short format.
- Reference ID (refid): 32-bit code identifying the particular server or reference clock (the interpretation depends on the value in the stratum field).
- Reference Timestamp (reftime): Time when the system clock was last set or corrected, in NTP timestamp format.
- Origin Timestamp (org): Time at the client when the request departed for the server, in NTP timestamp format.
- Recieve Timestamp (rec): Time at the server when the request arrived from the client, in NTP timestamp format.
- Transmit Timestamp (xmt): Time at the server when the response left for the client, in NTP timestamp format.
- Destination Timestamp (dst): Time at the client when the reply arrived from the server, in NTP timestamp format.
- Key Identifier (keyid): 32-bit unsigned integer used by the client and server to designate a secret 128-bit MD5 key.
- Message Digest (digest): 128-bit MD5 hash computed over the key followed by the NTP packet header and extensions fields (but not the Key Identifier or Message Digest fields).

5. If the Stratum field is 0, the Reference Identifier field can be used to convey messages useful for status reporting and access control. These are called Kiss-0´-Death (KoD) Packets, and the ASCII messages they convey are called kiss codes.

### 8. On-Wire Protocol

1. The core mechanism in NTP on-wire protocol excahnges time values between servers, peers and clients. It is resistant to lost or duplicate packets, data integrity is provided by the IP and UDP checksums, and no flow control or retransmission facilities are necessary.
2. NTP messages make use of two different communication modes, one-to-one (unicast) and one-to-many (broadcast).
3. The on-wire protocol uses four timestamps numbered t1 through t4 and three state variables org, rec and xmt (read more on page 28 in the NTP spec).
4. The four most recent timestamps, T1 through T4, are used to compute the offset of B relative to A:

<div style="border: 1px solid black; padding: 10px;">
  theta = T(B) - T(A) = 1/2 * [(T2-T1) + (T3-T4)]
</div>

and the round-trip delay:

<div style="border: 1px solid black; padding: 10px;">
  delta = T(ABA) = (T4-T1) - (T3-T2)
</div>

The quantities within parentheses are computed from 64-bit unsigned timestamps and result in signed values with 63 significant bits plus sign, which represents dates from 68 years in the past to 68 years in the future. However, the offset and delay contain 62 significant bits and two sign bits so they can represent 34 years in the past and 34 years in the future. The time of the client must therefore be set within 34 years of the server before the service is started. The unambiguous range can be restored from 34 to 68 years if the first-order differences can be converted to floating double and the second-order sums and differences computed in that arithmetic.

5. In some scenarios the delay computation may become negative, therefore the value of delta should be clamped not less than s.rho (the system precision) to get rid of the missleading delta values.
6. The on-wire protocol resists replay of a server response packet it does not resist replay of the client request packet, which would result in a server reply packet with new values of T2 and T3 and result in incorrect offset and delay. This can be avoided by setting the xmt variable to zero after computing the offset adn delay.

### 9. Peer Process

1. The peer process is called upon arrival of a server or peer packet and runs the on-wire protocol to determine the clock offset and roundtrip delay and computes statistics used by the system and poll process.
2. There is a peer process, poll process and association process for each server.
3. Peer Process Variables are specified on page 31-32 in the NTP spec.

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
