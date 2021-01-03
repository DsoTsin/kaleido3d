#include "metalib2spirv_p.h"
#include <algorithm>
#include <iterator>
#include <sstream>

namespace metal {
using namespace llvm;
String::String() {
  buffer = nullptr;
  length = 0;
  capacity = 0;
}

String::String(const char *str, size_t in_length) { assign(str, in_length); }

String::String(const StringRef &ref) {
  assign(ref.data(), ref.size());
}

String::~String() {
  if (buffer) {
    free(buffer);
    buffer = nullptr;
  }
  length = 0;
  capacity = 0;
}

void String::assign(const char *str, size_t len) {
  if (buffer) {
    free(buffer);
    buffer = nullptr;
  }
  length = len == 0 ? strlen(str) : len;
  capacity = length + 1;
  auto ptr = static_cast<char *>(malloc(capacity));
  memcpy(ptr, str, length);
  ptr[length] = 0;
  buffer = ptr;
}

static constexpr const char metallib_magic[4]{'M', 'T', 'L', 'B'};

LLVM_PACKED(struct MetalLibVersion {
  // container/file version
  uint16_t container_version_major : 15;
  uint16_t is_macos_target : 1;
  uint16_t container_version_minor;
  uint16_t container_version_bugfix;

  // flags
  uint8_t file_type : 7;
  uint8_t is_stub : 1;
  uint8_t platform : 7;
  uint8_t is_64_bit : 1;

  // platform version
  uint32_t platform_version_major : 16;
  uint32_t platform_version_minor : 8;
  uint32_t platform_version_update : 8;
});
static_assert(sizeof(MetalLibVersion) == 12, "invalid version header length");

LLVM_PACKED(struct MetalLibHeaderControl {
  uint64_t programs_offset;
  uint64_t programs_length;
  uint64_t reflection_offset;
  uint64_t reflection_length;
  uint64_t debug_offset;
  uint64_t debug_length;
  uint64_t bitcode_offset;
  uint64_t bitcode_length;
});
static_assert(sizeof(MetalLibHeaderControl) == 64,
              "invalid program info length");

LLVM_PACKED(struct MetalLibHeader {
  const char magic[4]; // == metallib_magic
  const MetalLibVersion version;
  const uint64_t file_length;
  const MetalLibHeaderControl header_control;
});
static_assert(sizeof(MetalLibHeader) == 4 + sizeof(MetalLibVersion) +
                                            sizeof(uint64_t) +
                                            sizeof(MetalLibHeaderControl),
              "invalid metallib header size");

struct LibraryMeta {
  uint32_t length; // including length itself

  // NOTE: tag types are always 32-bit
  // NOTE: tag types are always followed by a uint16_t that specifies the length
  // of the tag data
#define make_tag_type(a, b, c, d)                                              \
  ((uint32_t(d) << 24u) | (uint32_t(c) << 16u) | (uint32_t(b) << 8u) |         \
   uint32_t(a))
  enum Tag : uint32_t {
    // used in initial header section
    NAME = make_tag_type('N', 'A', 'M', 'E'),
    TYPE = make_tag_type('T', 'Y', 'P', 'E'),
    HASH = make_tag_type('H', 'A', 'S', 'H'),
    MD_SIZE = make_tag_type('M', 'D', 'S', 'Z'),
    OFFSET = make_tag_type('O', 'F', 'F', 'T'),
    VERSION = make_tag_type('V', 'E', 'R', 'S'),
    // used in reflection section
    CNST = make_tag_type('C', 'N', 'S', 'T'),
    VATT = make_tag_type('V', 'A', 'T', 'T'),
    VATY = make_tag_type('V', 'A', 'T', 'Y'),
    RETR = make_tag_type('R', 'E', 'T', 'R'),
    ARGR = make_tag_type('A', 'R', 'G', 'R'),
    // used in debug section
    DEBI = make_tag_type('D', 'E', 'B', 'I'),
    // TODO/TBD
    LAYR = make_tag_type('L', 'A', 'Y', 'R'),
    TESS = make_tag_type('T', 'E', 'S', 'S'),
    SOFF = make_tag_type('S', 'O', 'F', 'F'),
    // generic end tag
    END = make_tag_type('E', 'N', 'D', 'T'),
  };
#undef make_tag_type

  enum class FunctionType : uint8_t {
    VERTEX = 0,
    FRAGMENT = 1,
    KERNEL = 2,
    // TODO: tessellation?
    NONE = 255
  };

  struct VersionInfo {
    uint32_t major : 16;
    uint32_t minor : 8;
    uint32_t rev : 8;
  };

  struct Offset {
    // NOTE: these are all relative offsets -> add to MetalLibHeaderControl
    // offsets to get absolute offsets
    uint64_t reflection_offset;
    uint64_t debug_offset;
    uint64_t bitcode_offset;
  };

  struct Sha256 {
    uint8_t data[32]{
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
  };

  struct FunctionEntry {
    uint32_t length;
    llvm::StringRef name; // NOTE: limited to 65536 - 1 ('\0')
    FunctionType type{FunctionType::NONE};
    Sha256 sha256_hash;
    Offset offset{0, 0, 0};
    uint64_t bitcode_size{0}; // always 8 bytes
    VersionInfo api_version{0, 0, 0};
    VersionInfo language_version{0, 0, 0};
    uint8_t tess_info{0};
    uint64_t soffset{0};
  };

  llvm::SmallVector<FunctionEntry> entries;
}; // LibraryMeta

Expected<Library *> load_library(const uint8_t *bytes, size_t num_bytes,
                                 LLVMContext &context, raw_ostream &os) {
  if (num_bytes < sizeof(MetalLibHeader)) {
    return make_error<StringError>("invalid header size",
                                   inconvertibleErrorCode());
  }

  const auto &header = *(const MetalLibHeader *)bytes;
  if (memcmp(metal::metallib_magic, header.magic, 4) != 0) {
    return make_error<StringError>("invalid magic", inconvertibleErrorCode());
  }

  os << "[header]" << '\n';
  os << "container version: " << header.version.container_version_major << "."
     << header.version.container_version_minor << "."
     << header.version.container_version_bugfix << '\n';
  os << "macos? " << header.version.is_macos_target << '\n';
  os << "length: " << header.file_length << '\n';

  os << '\n';
  os << "programs_offset: " << header.header_control.programs_offset << '\n';
  os << "programs_length: " << header.header_control.programs_length << '\n';
  os << "reflection_offset: " << header.header_control.reflection_offset
     << '\n';
  os << "reflection_length: " << header.header_control.reflection_length
     << '\n';
  os << "debug_offset: " << header.header_control.debug_offset << '\n';
  os << "debug_length: " << header.header_control.debug_length << '\n';
  os << "bitcode_offset: " << header.header_control.bitcode_offset << '\n';
  os << "bitcode_length: " << header.header_control.bitcode_length << '\n';
  os << "program_count: " << header.header_control.programs_length << '\n';

  if (num_bytes < header.header_control.programs_offset +
                      header.header_control.programs_length + 4u) {
    return make_error<StringError>("invalid size", inconvertibleErrorCode());
  }

  LibraryMeta info;
  auto program_ptr = &bytes[header.header_control.programs_offset];
  const auto add_program_md_offset = header.header_control.programs_offset + 4 +
                                     header.header_control.programs_length;
  auto add_program_md_ptr = &bytes[add_program_md_offset];
  const auto add_program_md_end =
      std::find((const uint32_t *)add_program_md_ptr,
                +(const uint32_t *)add_program_md_ptr +
                    (header.header_control.reflection_offset -
                     +add_program_md_offset) /
                        4u,
                +0x54444E45u /* rev(ENDT) */) +
      1;
  const auto add_program_md_length =
      std::distance(add_program_md_ptr, (const uint8_t *)add_program_md_end);
  auto refl_ptr = &bytes[header.header_control.reflection_offset];
  auto debug_ptr = &bytes[header.header_control.debug_offset];
  const auto program_count = *(const uint32_t *)program_ptr;
  program_ptr += 4;
  os << "program_count: " << program_count << '\n';
  info.entries.resize(program_count);

  for (uint32_t i = 0; i < program_count; ++i) {
    auto &entry = info.entries[i];

    entry.length = *(const uint32_t *)program_ptr;
    program_ptr += 4;

    bool found_end_tag = false;
    while (!found_end_tag) {
      const auto tag = *(const LibraryMeta::Tag *)program_ptr;
      program_ptr += 4;
      uint32_t tag_length = 0;
      if (tag != LibraryMeta::END) {
        tag_length = *(const uint16_t *)program_ptr;
        program_ptr += 2;

        if (tag_length == 0) {
          return make_error<StringError>(
              "tag " + std::to_string(uint32_t(tag)) + " should not be empty",
              inconvertibleErrorCode());
        }
      }

      switch (tag) {
      case LibraryMeta::NAME: {
        entry.name =
            llvm::StringRef((const char *)program_ptr, tag_length - 1u);
        break;
      }
      case LibraryMeta::TYPE: {
        entry.type = *(const LibraryMeta::FunctionType *)program_ptr;
        break;
      }
      case LibraryMeta::HASH: {
        if (tag_length != 32) {
          return make_error<StringError>("invalid hash size: " +
                                             std::to_string(tag_length),
                                         inconvertibleErrorCode());
        }
        memcpy(&entry.sha256_hash, program_ptr, 32u);
        break;
      }
      case LibraryMeta::OFFSET: {
        entry.offset = *(const LibraryMeta::Offset *)program_ptr;
        break;
      }
      case LibraryMeta::VERSION: {
        entry.api_version = *(const LibraryMeta::VersionInfo *)program_ptr;
        entry.language_version =
            *((const LibraryMeta::VersionInfo *)program_ptr + 1u);
        break;
      }
      case LibraryMeta::MD_SIZE: {
        // TODO: this might not exist?
        entry.bitcode_size = *(const uint64_t *)program_ptr;
        break;
      }
      case LibraryMeta::TESS: {
        entry.tess_info = *(const uint8_t *)program_ptr;
        break;
      }
      case LibraryMeta::SOFF: {
        if (tag_length != 8) {
          return make_error<StringError>("invalid SOFF size: " +
                                             std::to_string(tag_length),
                                         inconvertibleErrorCode());
        }
        entry.soffset = *(const uint64_t *)program_ptr;
        break;
      }
      case LibraryMeta::END: {
        found_end_tag = true;
        break;
      }
      default:
        return make_error<StringError>("invalid tag: " +
                                           std::to_string((uint32_t)tag),
                                       inconvertibleErrorCode());
      }
      program_ptr += tag_length;
    }
  }
  if (info.entries.size() != program_count) {
    return make_error<StringError>("invalid entry count",
                                   inconvertibleErrorCode());
  }

  llvm::SmallVector<Library::Function, 10> functions;
  std::unordered_map<String, size_t> map;
  for (const auto &prog : info.entries) {
    os << '\n';
    os << "####################################################################"
          "############\n";
    os << '\n';
    os << "[program]" << '\n';
    os << "\tname: " << prog.name << '\n';
    os << "\ttype: ";
    switch (prog.type) {
    case LibraryMeta::FunctionType::FRAGMENT:
      os << "fragment";
      break;
    case LibraryMeta::FunctionType::VERTEX:
      os << "vertex";
      break;
    case LibraryMeta::FunctionType::KERNEL:
      os << "kernel";
      break;
    case LibraryMeta::FunctionType::NONE:
      os << "NONE";
      break;
    }
    os << '\n';
    os << "\tversion: " << prog.api_version.major << "."
       << prog.api_version.minor << "." << prog.api_version.rev << '\n';
    os << "\tlanguage: " << prog.language_version.major << "."
       << prog.language_version.minor << "." << prog.language_version.rev
       << '\n';
    os << "\trel offsets (refl, dbg, bc): " << prog.offset.reflection_offset
       << ", " << prog.offset.debug_offset << ", " << prog.offset.bitcode_offset
       << '\n';
    os << "\tbitcode size: " << prog.bitcode_size << '\n';
    os << "\thash: ";

    std::stringstream hash_hex;
    hash_hex << std::hex << std::uppercase;
    for (uint32_t i = 0; i < 32; ++i) {
      if (prog.sha256_hash.data[i] < 0x10) {
        hash_hex << '0';
      }
      hash_hex << uint32_t(prog.sha256_hash.data[i]);
    }
    os << hash_hex.str() << '\n';
    os << "\ttess info: " << uint32_t(prog.tess_info) << '\n';
    os << "\tsoffset: " << uint32_t(prog.soffset) << '\n';

    auto bc_mem = llvm::WritableMemoryBuffer::getNewUninitMemBuffer(
        prog.bitcode_size, "bc_module");
    const auto bc_offset =
        header.header_control.bitcode_offset + prog.offset.bitcode_offset;
    if (bc_offset + prog.bitcode_size > num_bytes) {
      return make_error<StringError>("invalid bitcode offset",
                                     inconvertibleErrorCode());
    }
    os << "\toffset: " << bc_offset << '\n';
    os << "\tsize: " << bc_mem->getBufferSize() << ", " << prog.bitcode_size
       << '\n';
    os << '\n';
    memcpy((char *)bc_mem->getBufferStart(), bytes + bc_offset,
           bc_mem->getBufferSize());

    auto bc_mod = llvm::parseBitcodeFile(*bc_mem, context);
    if (bc_mod) {
      auto error = bc_mod.get()->materializeAll();
      Library::Function function;
      function.entry_name = prog.name;
      map.insert({function.entry_name, functions.size()});
      function.shader_type = (mtl2spv_shader_type)prog.type;
      function.module = std::move(bc_mod.get());
      function.buffer = std::move(bc_mem);
      function.parse_arguments();
      function.sync_api();
      functions.push_back(std::move(function));
    } else {
      os << "bc parse error" << '\n';
      return make_error<StringError>("failed to parse bitcode module",
                                     inconvertibleErrorCode());
    }
  }
  if (info.entries.size() == functions.size()) {
    auto lib = new Library;
    lib->functions = std::move(functions);
    lib->map = std::move(map);
    return lib;
  } else {
    return nullptr;
  }
}
} // namespace metal
