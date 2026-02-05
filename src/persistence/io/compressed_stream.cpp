#include "persistence/io/compressed_stream.h"
#include <fstream>
#include <zlib.h>
#include <iostream>
#include <iterator>

namespace voxeng {
namespace io {

// =============================================================================
// CompressedFileWriter
// =============================================================================

CompressedFileWriter::CompressedFileWriter(std::string file_path)
    : m_file_path(std::move(file_path)) {}

bool CompressedFileWriter::save() {
    // 1. Compress
    uLongf compressed_size = compressBound(m_buffer.size());
    std::vector<char> compressed_data(compressed_size);

    int result = ::compress(
        reinterpret_cast<Bytef*>(compressed_data.data()), &compressed_size,
        reinterpret_cast<const Bytef*>(m_buffer.data()), m_buffer.size()
    );

    if (result != Z_OK) {
        std::cerr << "CompressedFileWriter: compression failed. Error: " << result << "\n";
        return false;
    }

    compressed_data.resize(compressed_size);

    // 2. Write to Disk
    std::ofstream file(m_file_path, std::ios::binary);
    if (!file) {
        std::cerr << "CompressedFileWriter: failed to open file " << m_file_path << "\n";
        return false;
    }

    uint64_t original_size = m_buffer.size();
    file.write(reinterpret_cast<const char*>(&original_size), sizeof(original_size));
    file.write(compressed_data.data(), compressed_data.size());

    // Optional: Log success
    // std::cout << "Saved compressed file: " << m_file_path << "\n";

    clear();
    return true;
}

void CompressedFileWriter::clear() {
    m_buffer.clear();
}

// =============================================================================
// CompressedFileReader
// =============================================================================

CompressedFileReader::CompressedFileReader(std::string file_path)
    : m_file_path(std::move(file_path)) {}

bool CompressedFileReader::load() {
    std::ifstream file(m_file_path, std::ios::binary);

    if (!file) {
        std::cerr << "CompressedFileReader: failed to open file " << m_file_path << "\n";
        return false;
    }

    // 1. Read Header (Original Size)
    uint64_t uncompressed_size = 0;
    file.read(reinterpret_cast<char*>(&uncompressed_size), sizeof(uncompressed_size));

    if (file.fail()) {
        std::cerr << "CompressedFileReader: failed to read header.\n";
        return false;
    }

    // 2. Read Compressed Data
    std::vector<char> compressed_data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // 3. Decompress
    m_buffer.resize(uncompressed_size);
    uLongf dest_len = uncompressed_size;

    int result = ::uncompress(
        reinterpret_cast<Bytef*>(m_buffer.data()), &dest_len,
        reinterpret_cast<const Bytef*>(compressed_data.data()), compressed_data.size()
    );

    if (result != Z_OK) {
        std::cerr << "CompressedFileReader: decompression failed. Error: " << result << "\n";
        return false;
    }

    return true;
}

bool CompressedFileReader::isCursorAtEnd() const {
    return m_cursor >= m_buffer.size();
}

void CompressedFileReader::resetCursor() {
    m_cursor = 0;
}

void CompressedFileReader::clear() {
    m_buffer.clear();
    m_cursor = 0;
}

} // namespace io
} // namespace voxeng
