#include "jpegheaderreader.h"
#include <cstring>

// Helper to read big-endian uint16
static uint16_t read_u16_be(const uint8_t* ptr) {
    return (uint16_t)((ptr[0] << 8) | ptr[1]);
}

JpegHeaderReader::JpegHeaderReader(const uint8_t* data, size_t size) 
    : m_data(data), m_size(size), m_pos(0)
{
    readMark();
}

uint16_t JpegHeaderReader::current() {
    return currentMark;
}

uint16_t JpegHeaderReader::size() {
    return currentSize;
}

std::vector<uint8_t> JpegHeaderReader::read() {
    std::vector<uint8_t> buffer;
    if (valid() && currentSize > 0) {
        // currentSize includes the 2 bytes for length itself?
        // In JPEG, the length field (2 bytes) includes itself.
        // But the marker itself (2 bytes) is not included.
        // My interface seems to imply 'read()' returns the content of the segment.
        // The previous Qt implementation logic was:
        // skip() moves past the segment.
        
        // Let's look at the previous usage:
        // if (jpegReader.current() == M_APP1) { exif += jpegReader.read(); }
        // So read() should return the data payload.
        
        // The length field (2 bytes) at m_pos is part of the payload technically in some views, 
        // but typically "length" includes those 2 bytes.
        // The payload size is length - 2.
        
        if (currentSize >= 2) {
             // We need to read currentSize - 2 bytes.
             // But wait, the previous code in jpegavifconverter.cpp used:
             // jpegFile.write( ... header ... )
             // jpegFile.write( ... content ... )
             
             // Let's implement read() to return the WHOLE segment data excluding marker, 
             // but INCLUDING the length bytes? 
             // Re-checking previous code:
             // get_word(i+2) is length.
             // write(..., length + 2) which includes marker(2) + length(2) + payload.
             
             // However, JpegHeaderReader::read() returned QByteArray.
             // In previous jpegavifconverter.cpp:
             // exif += jpegReader.read();
             // (void)avifImageSetMetadataExif(..., exif.data(), ...)
             
             // This implies `read()` returns the raw EXIF payload (likely excluding length bytes? or including?)
             // Usually libavif expects raw EXIF blob (starting with Exif\0\0...). 
             // In JPEG APP1, it is Marker(2) + Length(2) + Payload. 
             // Payload starts with Exif...
             
             // Let's assume read() returns the payload (excluding length bytes).
             // But wait, `currentSize` is read from the 2 bytes after marker.
             // It includes the 2 bytes of length.
             
             size_t payloadSize = currentSize - 2;
             if (m_pos + 2 + payloadSize <= m_size) {
                 buffer.resize(payloadSize);
                 memcpy(buffer.data(), m_data + m_pos + 2, payloadSize);
                 m_pos += 2 + payloadSize;
                 readMark();
             }
        }
    }
    return buffer;
}

void JpegHeaderReader::skip() {
    if (currentMark == M_SOS || currentMark == M_EOI) {
        return; 
    }
    // currentSize includes the 2 length bytes.
    // m_pos is at the length bytes.
    if (m_pos + currentSize <= m_size) {
        m_pos += currentSize;
        readMark();
    } else {
        m_pos = m_size; // End
    }
}

bool JpegHeaderReader::valid() {
    return m_pos < m_size;
}

bool JpegHeaderReader::atEnd() {
    return m_pos >= m_size || currentMark == M_EOI || currentMark == M_SOS; 
    // M_SOS starts entropy data, we stop there for header parsing.
}

void JpegHeaderReader::readMark() {
    if (m_pos + 2 > m_size) {
        m_pos = m_size;
        return;
    }

    uint16_t marker = read_u16_be(m_data + m_pos);
    
    if (marker != M_SOI && (marker & 0xFF00) != 0xFF00) {
        // Sync issue or invalid
        // Try to find next FF
        m_pos++;
        readMark(); 
        return;
    }

    currentMark = marker;
    m_pos += 2;

    // Some markers don't have size
    if (marker == M_SOI || marker == M_EOI || marker == M_TEM || (marker >= M_RST0 && marker <= M_RST7)) {
        currentSize = 0;
    } else {
        if (m_pos + 2 <= m_size) {
            currentSize = read_u16_be(m_data + m_pos);
        } else {
            currentSize = 0;
        }
    }
}