#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include "Stream.h"

#define PACKET_HEADER_1 0XAA
#define PACKET_HEADER_2 0X55
#define PACKET_TYPE_CTR 0x01

typedef struct __attribute__((packed)) {
    uint8_t header1;
    uint8_t header2;
    uint8_t type;
    uint8_t detected;
    int16_t error;
    uint8_t checksum;
} Packet;

bool sendPacket(Packet& packet, Stream& stream);
bool receivePacket(Packet& packet, Stream& stream);
uint8_t calculateChecksum(const Packet& packet);

#endif
