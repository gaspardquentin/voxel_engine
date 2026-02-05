#pragma once

#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <cstdint>

namespace voxeng {
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

    bool isCursorAtEnd() const;
    void resetCursor();
    void clear();
};

} // namespace io
} // namespace voxeng

