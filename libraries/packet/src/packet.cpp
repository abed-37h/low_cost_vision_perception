#include "packet.h"

bool sendPacket(Packet& packet, Stream& stream) {
    packet.header1  = PACKET_HEADER_1;
    packet.header2  = PACKET_HEADER_2;
    packet.type     = PACKET_TYPE_CTR;
    packet.checksum = calculateChecksum(packet);

    size_t n = stream.write((uint8_t*)&packet, sizeof(packet));
    return n == sizeof(Packet);
}

bool receivePacket(Packet& packet, Stream& stream) {
    static uint8_t state = 0;
    static Packet p;

    while (stream.available()) {
        uint8_t b = stream.read();

        if (state == 0) {
            if (b == PACKET_HEADER_1) {
                ((uint8_t*) &p)[state++] = b;
            }
            continue;
        }

        if (state == 1) {
            if (b == PACKET_HEADER_2) {
                ((uint8_t*) &p)[state++] = b;
            }
            else if (b == PACKET_HEADER_1) {
                ((uint8_t*) &p)[0] = b;
            }
            else {
                state = 0;
            }
            continue;
        }

        ((uint8_t*)&p)[state++] = b;

        if (state == sizeof(Packet)) {
            state = 0;
            
            if (
                p.type != PACKET_TYPE_CTR
                || p.checksum != calculateChecksum(p)
            ) return false;
        
            packet = p;
            return true;
        }
    }
    
    return false;
}

uint8_t calculateChecksum(const Packet& packet) {
    return packet.type ^
           packet.detected ^
           (packet.error & 0xFF) ^
           ((packet.error >> 8) & 0xFF);
}