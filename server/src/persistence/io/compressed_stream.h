#pragma once

#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <cstdint>

namespace voxeng::server {
namespace io {

class CompressedFileWriter {
    std::string m_file_path;
    std::vector<char> m_buffer;

public:
    explicit CompressedFileWriter(std::string file_path);

    template <typename T>
    void write(const T& data) {
        const char *bytes = reinterpret_cast<const char*>(&data);
        m_buffer.insert(m_buffer.end(), bytes, bytes + sizeof(T));
    }

    void writeString(const std::string& str) {
        uint16_t len = static_cast<uint16_t>(str.size());
        write(len);
        m_buffer.insert(m_buffer.end(), str.begin(), str.end());
    }

    /*
     * Compresses the internal buffer and writes it to the disk.
     * Returns true if successful.
     */
    bool save();

    void clear();
};

class CompressedFileReader {
    std::string m_file_path;
    std::vector<char> m_buffer;
    size_t m_cursor = 0;

public:
    explicit CompressedFileReader(std::string file_path);

    /*
     * Reads the file, decompresses it into the internal buffer.
     * Returns true if successful.
     */
    bool load();

    template <typename T>
    void read(T& out) {
        if (m_cursor + sizeof(T) > m_buffer.size()) {
            std::cerr << "CompressedFileReader: attempt to read past end of buffer.\n";
            return;
        }
        std::memcpy(&out, &m_buffer[m_cursor], sizeof(T));
        m_cursor += sizeof(T);
    }

    void readString(std::string& out) {
        uint16_t len = 0;
        read(len);
        if (m_cursor + len > m_buffer.size()) {
            std::cerr << "CompressedFileReader: attempt to read string past end of buffer.\n";
            return;
        }
        out.assign(&m_buffer[m_cursor], len);
        m_cursor += len;
    }

    bool isCursorAtEnd() const;
    void resetCursor();
    void clear();
};

}
}
