package main

import (
	"encoding/binary"
	"fmt"
	"net"
	"os"
	"os/signal"
	"syscall"
	"time"
)

const (
	NTP_EPOCH_OFFSET = 2208988800 // Time offset between 1900 and 1970 (Unix Epoch)
)

type NTPPacket struct {
	LiVnMode       byte
	Stratum        byte
	Poll           byte
	Precision      byte
	RootDelay      uint32
	RootDispersion uint32
	ReferenceID    uint32
	RefTimestamp   uint64
	OrigTimestamp  uint64
	RxTimestamp    uint64
	TxTimestamp    uint64
}

func corruptNTPPacket(packet *NTPPacket) {
	packet.Stratum = 17 // Invalid stratum
}

func handleConnection(conn *net.UDPConn, fixedTime uint64, corrupt bool) {
	buffer := make([]byte, 48)
	for {
		_, addr, err := conn.ReadFromUDP(buffer)
		if err != nil {
			fmt.Println("Error reading from UDP:", err)
			continue
		}

		// Correct LiVnMode field
		li := byte(0)   // Leap Indicator: 0
		vn := byte(4)   // Version Number: 4
		mode := byte(4) // Mode: 4 (Server)
		liVnMode := li<<6 | vn<<3 | mode

		packet := &NTPPacket{
			LiVnMode:    liVnMode,
			Stratum:     1,
			Precision:   0xFA,
			RxTimestamp: fixedTime,
			TxTimestamp: fixedTime,
		}

		if corrupt {
			corruptNTPPacket(packet)
		}

		buffer[0] = packet.LiVnMode
		buffer[1] = packet.Stratum
		buffer[2] = packet.Poll
		buffer[3] = packet.Precision
		binary.BigEndian.PutUint32(buffer[32:], uint32(packet.RxTimestamp>>32)) // Seconds
		binary.BigEndian.PutUint32(buffer[36:], uint32(packet.RxTimestamp))     // Fractional seconds
		binary.BigEndian.PutUint32(buffer[40:], uint32(packet.TxTimestamp>>32)) // Seconds
		binary.BigEndian.PutUint32(buffer[44:], uint32(packet.TxTimestamp))     // Fractional seconds

		_, err = conn.WriteToUDP(buffer, addr)
		if err != nil {
			fmt.Println("Failed to send UDP:", err)
		}
	}
}

func main() {
	port := ":123"
	corrupt := false
	args := os.Args[1:]
	if len(args) > 0 && args[0] == "--corrupt" {
		corrupt = true
	}

	addr, err := net.ResolveUDPAddr("udp", port)
	if err != nil {
		fmt.Println("Failed to resolve UDP address:", err)
		return
	}

	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		fmt.Println("Failed to listen on UDP:", err)
		return
	}
	defer conn.Close()

	fmt.Println("NTP Server started on", port)

	fixedDate := time.Date(2002, time.June, 23, 6, 30, 0, 0, time.UTC)
	fixedUnix := fixedDate.Unix()
	fixedNTP := (uint64(fixedUnix) + NTP_EPOCH_OFFSET) << 32 // Shift left to put into the upper 32 bits

	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt, syscall.SIGTERM)
	go func() {
		<-c
		fmt.Println("\nNTP Server shutting down")
		conn.Close()
		os.Exit(0)
	}()

	handleConnection(conn, fixedNTP, corrupt)
}
