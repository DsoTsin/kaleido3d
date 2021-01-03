#include "metalib2spirv_p.h"

#if _MSC_VER >= 1932 // Visual Studio 2022 version 17.2+
#pragma comment(linker, "/alternatename:__imp___std_init_once_complete=__imp_InitOnceComplete")
#pragma comment(linker, "/alternatename:__imp___std_init_once_begin_initialize=__imp_InitOnceBeginInitialize")
#endif

namespace {
std::string consume_error_message(llvm::Error error) {
  std::string message;
  llvm::handleAllErrors(std::move(error), [&](const llvm::ErrorInfoBase &info) {
    message = info.message();
  });
  return message;
}

mtl2spv_error map_loader_error(const std::string &message) {
  if (message.find("invalid header size") != std::string::npos) {
    return MTL2SPV_LDR_INVALID_HEADER_SIZE;
  }
  if (message.find("invalid magic") != std::string::npos) {
    return MTL2SPV_LDR_INVALID_HEADER_MAGIC;
  }
  if (message.find("invalid size") != std::string::npos) {
    return MTL2SPV_LDR_INVALID_LIBRARY_SIZE;
  }
  if (message.find("invalid hash size") != std::string::npos) {
    return MTL2SPV_LDR_INVALID_PROGRAM_HASH_SIZE;
  }
  if (message.find("invalid bitcode offset") != std::string::npos) {
    return MTL2SPV_LDR_INVALID_PROGRAM_OFFSET;
  }
  if (message.find("invalid tag") != std::string::npos ||
      message.find("should not be empty") != std::string::npos) {
    return MTL2SPV_LDR_INVALID_TAG;
  }
  if (message.find("invalid entry count") != std::string::npos) {
    return MTL2SPV_LDR_INVALID_ENTRY_COUNT;
  }
  if (message.find("failed to parse bitcode module") != std::string::npos) {
    return MTL2SPV_LDR_FAILED_TO_PARSE_BC;
  }
  return MTL2SPV_LDR_PARSE_PROGRAM_ERROR;
}
} // namespace

namespace metal {
Library::Library() {}
Library::~Library() {}

int Library::count() const { return (int)functions.size(); }

const char *Library::get_name(int index) const {
  return index >= 0 && index < (int)functions.size()
             ? functions[index].entry_name.c_str()
             : nullptr;
}

mtl2spv_shader_type Library::get_type(int index) const {
  return index >= 0 && index < (int)functions.size()
             ? functions[index].shader_type
             : MTL2SPV_SHADER_UNKNOWN;
}

const mtl2spv_function *Library::get_function(int index) const {
  return index >= 0 && index < (int)functions.size() ? &functions[index].api
                                                     : nullptr;
}
} // namespace metal

extern "C" {
mtl2spv_error metallib_open(const void *bytes, size_t num_bytes,
                            metallib_t *olib) {
  static llvm::LLVMContext context;
  std::string trace;
  llvm::raw_string_ostream trace_stream(trace);
  auto lib =
      metal::load_library((const uint8_t *)bytes, num_bytes, context, trace_stream);
  if (!lib) {
    return map_loader_error(consume_error_message(lib.takeError()));
  }
  if (!olib) {
    delete *lib;
    return MTL2SPV_LDR_PARSE_PROGRAM_ERROR;
  }

  *olib = (metallib_t)*lib;
  return MTL2SPV_SUCCESS;
}

void metallib_free(metallib_t lib) {
  if (lib) {
    delete (metal::Library *)lib;
  }
}

int metallib_function_count(metallib_t lib) {
  return lib ? ((metal::Library *)lib)->count() : 0;
}

const char *metallib_function_name(metallib_t lib, int index) {
  return lib ? ((metal::Library *)lib)->get_name(index) : nullptr;
}

const mtl2spv_function *metallib_function(metallib_t lib, int index) {
  return lib ? ((metal::Library *)lib)->get_function(index) : nullptr;
}

mtl2spv_shader_type metallib_function_type(metallib_t lib, int index) {
  return lib ? ((metal::Library *)lib)->get_type(index)
             : MTL2SPV_SHADER_UNKNOWN;
}

void metallib2spv(metallib_t lib, const mtl2spv_options *options, int index) {
  if (!lib) {
    return;
  }

  const mtl2spv_options default_options{};
  ((metal::Library *)lib)->write_spv(options ? *options : default_options, index);
}

void metallib2spv2(metallib_t lib, const mtl2spv_options *options,
                   const char *entry_name) {
  if (!lib || !entry_name) {
    return;
  }

  const mtl2spv_options default_options{};
  ((metal::Library *)lib)->write_spv(options ? *options : default_options,
                                     metal::String(entry_name));
}

const uint32_t *metallib_spirv_data(metallib_t lib, int index,
                                    size_t *word_count) {
  return lib ? ((metal::Library *)lib)->get_spirv(index, word_count) : nullptr;
}

size_t metallib_spirv_word_count(metallib_t lib, int index) {
  return lib ? ((metal::Library *)lib)->get_spirv_word_count(index) : 0;
}

const char *metallib_last_error(metallib_t lib) {
  return lib ? ((metal::Library *)lib)->get_last_error() : nullptr;
}
}
