#include "spirv_builder.h"

#include <cassert>
#include <cstring>

namespace spv {

enum class VersionNumber : uint32_t {
  SPIRV_1_0 = 0x00010000,
  SPIRV_1_1 = 0x00010100,
};

static uint32_t get_version_word(unsigned int vk_version) {
  switch (vk_version) {
  case 11:
  case 0x00010100:
    return static_cast<uint32_t>(VersionNumber::SPIRV_1_1);
  case 10:
  case 0:
  case 0x00010000:
  default:
    return static_cast<uint32_t>(VersionNumber::SPIRV_1_0);
  }
}

Builder::Builder(const mtl2spv_options &in_options,
                 const metal::String &in_entry_name,
                 const mtl2spv_shader_type &in_shader_type, Blob &in_blob)
    : blob(in_blob), options(in_options), entry_name(in_entry_name),
      shader_type(in_shader_type), next_id(1), module_finalized(false),
      void_type_id(0), bool_type_id(0), sampler_type_id(0) {}

Id Builder::make_id() { return next_id++; }

// ---- Instruction encoding ----

void Builder::append_inst(std::vector<uint32_t> &section, Op op,
                          std::initializer_list<uint32_t> operands) {
  section.push_back((uint32_t(1 + operands.size()) << 16u) | uint16_t(op));
  section.insert(section.end(), operands.begin(), operands.end());
}

void Builder::append_inst(std::vector<uint32_t> &section, Op op,
                          const std::vector<uint32_t> &operands) {
  section.push_back((uint32_t(1 + operands.size()) << 16u) | uint16_t(op));
  section.insert(section.end(), operands.begin(), operands.end());
}

void Builder::append_string(std::vector<uint32_t> &words,
                            const metal::String &value) {
  const auto char_count = value.length + 1;
  const auto word_count = (char_count + 3u) / 4u;
  const auto base_index = words.size();
  words.resize(base_index + word_count, 0);
  memcpy(words.data() + base_index, value.c_str(), value.length);
}

void Builder::append_string(std::vector<uint32_t> &words, const char *value) {
  const auto len = strlen(value);
  const auto char_count = len + 1;
  const auto word_count = (char_count + 3u) / 4u;
  const auto base_index = words.size();
  words.resize(base_index + word_count, 0);
  memcpy(words.data() + base_index, value, len);
}

// ---- Type creation ----

Id Builder::void_type() {
  if (void_type_id == 0) {
    void_type_id = make_id();
    append_inst(types_globals, OpTypeVoid, {void_type_id});
  }
  return void_type_id;
}

Id Builder::bool_type() {
  if (bool_type_id == 0) {
    bool_type_id = make_id();
    append_inst(types_globals, OpTypeBool, {bool_type_id});
  }
  return bool_type_id;
}

Id Builder::int_type(unsigned width, bool is_signed) {
  TypeKey key{uint64_t(1) << 32 | width, is_signed ? 1u : 0u};
  auto it = type_cache.find(key);
  if (it != type_cache.end())
    return it->second;
  auto id = make_id();
  append_inst(types_globals, OpTypeInt, {id, width, is_signed ? 1u : 0u});
  type_cache[key] = id;
  int_signedness[id] = is_signed;
  return id;
}

Id Builder::float_type(unsigned width) {
  TypeKey key{uint64_t(2) << 32 | width, 0};
  auto it = type_cache.find(key);
  if (it != type_cache.end())
    return it->second;
  auto id = make_id();
  append_inst(types_globals, OpTypeFloat, {id, width});
  type_cache[key] = id;
  return id;
}

Id Builder::vector_type(Id component_type, unsigned count) {
  TypeKey key{uint64_t(3) << 32 | component_type, count};
  auto it = type_cache.find(key);
  if (it != type_cache.end())
    return it->second;
  auto id = make_id();
  append_inst(types_globals, OpTypeVector, {id, component_type, count});
  type_cache[key] = id;
  return id;
}

Id Builder::matrix_type(Id column_type, unsigned column_count) {
  TypeKey key{uint64_t(4) << 32 | column_type, column_count};
  auto it = type_cache.find(key);
  if (it != type_cache.end())
    return it->second;
  auto id = make_id();
  append_inst(types_globals, OpTypeMatrix, {id, column_type, column_count});
  type_cache[key] = id;
  return id;
}

Id Builder::array_type(Id element_type, Id length_id) {
  auto id = make_id();
  append_inst(types_globals, OpTypeArray, {id, element_type, length_id});
  return id;
}

Id Builder::runtime_array_type(Id element_type) {
  auto id = make_id();
  append_inst(types_globals, OpTypeRuntimeArray, {id, element_type});
  return id;
}

Id Builder::struct_type(const std::vector<Id> &member_types) {
  auto id = make_id();
  std::vector<uint32_t> ops = {id};
  ops.insert(ops.end(), member_types.begin(), member_types.end());
  append_inst(types_globals, OpTypeStruct, ops);
  return id;
}

Id Builder::pointer_type(Id pointee_type, StorageClass storage) {
  TypeKey key{uint64_t(6) << 32 | pointee_type, uint64_t(storage)};
  auto it = type_cache.find(key);
  if (it != type_cache.end())
    return it->second;
  auto id = make_id();
  append_inst(types_globals, OpTypePointer,
              {id, uint32_t(storage), pointee_type});
  type_cache[key] = id;
  return id;
}

Id Builder::function_type(Id return_type, const std::vector<Id> &param_types) {
  auto id = make_id();
  std::vector<uint32_t> ops = {id, return_type};
  ops.insert(ops.end(), param_types.begin(), param_types.end());
  append_inst(types_globals, OpTypeFunction, ops);
  return id;
}

Id Builder::image_type(Id sampled_type, Dim dim, uint32_t depth,
                       uint32_t arrayed, uint32_t ms, uint32_t sampled,
                       ImageFormat format) {
  auto id = make_id();
  append_inst(types_globals, OpTypeImage,
              {id, sampled_type, uint32_t(dim), depth, arrayed, ms, sampled,
               uint32_t(format)});
  return id;
}

Id Builder::sampled_image_type(Id image_type_id) {
  TypeKey key{uint64_t(8) << 32 | image_type_id, 0};
  auto it = type_cache.find(key);
  if (it != type_cache.end())
    return it->second;
  auto id = make_id();
  append_inst(types_globals, OpTypeSampledImage, {id, image_type_id});
  type_cache[key] = id;
  return id;
}

Id Builder::sampler_type() {
  if (sampler_type_id == 0) {
    sampler_type_id = make_id();
    append_inst(types_globals, OpTypeSampler, {sampler_type_id});
  }
  return sampler_type_id;
}

// ---- Constants ----

Id Builder::bool_constant(bool value) {
  uint64_t key = value ? 1 : 0;
  auto it = const_cache.find(key);
  if (it != const_cache.end())
    return it->second;
  auto id = make_id();
  append_inst(types_globals, value ? OpConstantTrue : OpConstantFalse,
              {bool_type(), id});
  const_cache[key] = id;
  return id;
}

Id Builder::int_constant(Id type, int32_t value) {
  uint64_t key = (uint64_t(type) << 32) | uint32_t(value);
  auto it = const_cache.find(key | (uint64_t(0x10) << 56));
  if (it != const_cache.end())
    return it->second;
  auto id = make_id();
  append_inst(types_globals, OpConstant, {type, id, uint32_t(value)});
  const_cache[key | (uint64_t(0x10) << 56)] = id;
  return id;
}

Id Builder::uint_constant(Id type, uint32_t value) {
  uint64_t key = (uint64_t(type) << 32) | value;
  auto it = const_cache.find(key | (uint64_t(0x20) << 56));
  if (it != const_cache.end())
    return it->second;
  auto id = make_id();
  append_inst(types_globals, OpConstant, {type, id, value});
  const_cache[key | (uint64_t(0x20) << 56)] = id;
  return id;
}

Id Builder::int64_constant(Id type, int64_t value) {
  auto id = make_id();
  uint32_t lo = uint32_t(value);
  uint32_t hi = uint32_t(uint64_t(value) >> 32);
  append_inst(types_globals, OpConstant, {type, id, lo, hi});
  return id;
}

Id Builder::uint64_constant(Id type, uint64_t value) {
  auto id = make_id();
  uint32_t lo = uint32_t(value);
  uint32_t hi = uint32_t(value >> 32);
  append_inst(types_globals, OpConstant, {type, id, lo, hi});
  return id;
}

Id Builder::float_constant(Id type, float value) {
  uint32_t bits;
  memcpy(&bits, &value, sizeof(bits));
  uint64_t key = (uint64_t(type) << 32) | bits;
  auto it = const_cache.find(key | (uint64_t(0x30) << 56));
  if (it != const_cache.end())
    return it->second;
  auto id = make_id();
  append_inst(types_globals, OpConstant, {type, id, bits});
  const_cache[key | (uint64_t(0x30) << 56)] = id;
  return id;
}

Id Builder::double_constant(Id type, double value) {
  uint64_t bits;
  memcpy(&bits, &value, sizeof(bits));
  auto id = make_id();
  uint32_t lo = uint32_t(bits);
  uint32_t hi = uint32_t(bits >> 32);
  append_inst(types_globals, OpConstant, {type, id, lo, hi});
  return id;
}

Id Builder::null_constant(Id type) {
  auto id = make_id();
  append_inst(types_globals, OpConstantNull, {type, id});
  return id;
}

Id Builder::composite_constant(Id type, const std::vector<Id> &constituents) {
  auto id = make_id();
  std::vector<uint32_t> ops = {type, id};
  ops.insert(ops.end(), constituents.begin(), constituents.end());
  append_inst(types_globals, OpConstantComposite, ops);
  return id;
}

// ---- Variables ----

Id Builder::global_variable(Id ptr_type, StorageClass storage,
                            Id initializer) {
  auto id = make_id();
  if (initializer != 0) {
    append_inst(types_globals, OpVariable,
                {ptr_type, id, uint32_t(storage), initializer});
  } else {
    append_inst(types_globals, OpVariable, {ptr_type, id, uint32_t(storage)});
  }
  return id;
}

Id Builder::function_variable(Id ptr_type) {
  auto id = make_id();
  append_inst(functions, OpVariable,
              {ptr_type, id, uint32_t(StorageClassFunction)});
  return id;
}

// ---- Extended instruction sets ----

Id Builder::import_ext_inst_set(const char *name) {
  auto id = make_id();
  std::vector<uint32_t> ops = {id};
  append_string(ops, name);
  append_inst(ext_imports, OpExtInstImport, ops);
  return id;
}

// ---- Decorations ----

void Builder::decorate(Id target, Decoration decoration,
                       const std::vector<uint32_t> &literals) {
  std::vector<uint32_t> ops = {target, uint32_t(decoration)};
  ops.insert(ops.end(), literals.begin(), literals.end());
  append_inst(annotations, OpDecorate, ops);
}

void Builder::member_decorate(Id struct_type_id, uint32_t member,
                              Decoration decoration,
                              const std::vector<uint32_t> &literals) {
  std::vector<uint32_t> ops = {struct_type_id, member, uint32_t(decoration)};
  ops.insert(ops.end(), literals.begin(), literals.end());
  append_inst(annotations, OpMemberDecorate, ops);
}

// ---- Module-level declarations ----

void Builder::add_capability(Capability capability) {
  append_inst(capabilities, OpCapability, {uint32_t(capability)});
}

void Builder::set_memory_model(AddressingModel addressing_model,
                               MemoryModel memory_model) {
  append_inst(memory_model_section, OpMemoryModel,
              {uint32_t(addressing_model), uint32_t(memory_model)});
}

void Builder::add_entry_point(ExecutionModel model, Id function_id,
                              const metal::String &name,
                              const std::vector<Id> &interface_ids) {
  std::vector<uint32_t> operands = {uint32_t(model), function_id};
  append_string(operands, name);
  for (auto id : interface_ids)
    operands.push_back(id);
  append_inst(entry_points, OpEntryPoint, operands);
}

void Builder::add_execution_mode(Id entry_point, ExecutionMode mode,
                                 std::initializer_list<uint32_t> literals) {
  std::vector<uint32_t> operands = {entry_point, uint32_t(mode)};
  operands.insert(operands.end(), literals.begin(), literals.end());
  append_inst(execution_modes, OpExecutionMode, operands);
}

void Builder::add_name(Id target_id, const metal::String &name) {
  std::vector<uint32_t> operands = {target_id};
  append_string(operands, name);
  append_inst(debug, OpName, operands);
}

void Builder::add_name(Id target_id, const char *name) {
  std::vector<uint32_t> operands = {target_id};
  append_string(operands, name);
  append_inst(debug, OpName, operands);
}

void Builder::add_member_name(Id struct_type_id, uint32_t member,
                              const char *name) {
  std::vector<uint32_t> operands = {struct_type_id, member};
  append_string(operands, name);
  append_inst(debug, OpMemberName, operands);
}

// ---- Function structure ----

Id Builder::begin_function(Id return_type, FunctionControlMask control,
                           Id fn_type) {
  auto id = make_id();
  append_inst(functions, OpFunction,
              {return_type, id, uint32_t(control), fn_type});
  return id;
}

Id Builder::begin_block() {
  auto id = make_id();
  append_inst(functions, OpLabel, {id});
  return id;
}

void Builder::set_block(Id label_id) {
  append_inst(functions, OpLabel, {label_id});
}

void Builder::end_function() { append_inst(functions, OpFunctionEnd, {}); }

Id Builder::begin_function_param(Id param_type) {
  auto id = make_id();
  append_inst(functions, OpFunctionParameter, {param_type, id});
  return id;
}

// ---- Instructions ----

Id Builder::emit_load(Id result_type, Id pointer, uint32_t memory_access) {
  auto id = make_id();
  if (memory_access != 0) {
    append_inst(functions, OpLoad, {result_type, id, pointer, memory_access});
  } else {
    append_inst(functions, OpLoad, {result_type, id, pointer});
  }
  return id;
}

void Builder::emit_store(Id pointer, Id value, uint32_t memory_access) {
  if (memory_access != 0) {
    append_inst(functions, OpStore, {pointer, value, memory_access});
  } else {
    append_inst(functions, OpStore, {pointer, value});
  }
}

Id Builder::emit_access_chain(Id result_type, Id base,
                              const std::vector<Id> &indexes) {
  auto id = make_id();
  std::vector<uint32_t> ops = {result_type, id, base};
  ops.insert(ops.end(), indexes.begin(), indexes.end());
  append_inst(functions, OpAccessChain, ops);
  return id;
}

Id Builder::emit_in_bounds_access_chain(Id result_type, Id base,
                                         const std::vector<Id> &indexes) {
  auto id = make_id();
  std::vector<uint32_t> ops = {result_type, id, base};
  ops.insert(ops.end(), indexes.begin(), indexes.end());
  append_inst(functions, OpInBoundsAccessChain, ops);
  return id;
}

Id Builder::emit_binop(Op opcode, Id result_type, Id operand1, Id operand2) {
  auto id = make_id();
  append_inst(functions, opcode, {result_type, id, operand1, operand2});
  return id;
}

Id Builder::emit_unop(Op opcode, Id result_type, Id operand) {
  auto id = make_id();
  append_inst(functions, opcode, {result_type, id, operand});
  return id;
}

Id Builder::emit_ternop(Op opcode, Id result_type, Id a, Id b, Id c) {
  auto id = make_id();
  append_inst(functions, opcode, {result_type, id, a, b, c});
  return id;
}

Id Builder::emit_composite_extract(Id result_type, Id composite,
                                   const std::vector<uint32_t> &indexes) {
  auto id = make_id();
  std::vector<uint32_t> ops = {result_type, id, composite};
  ops.insert(ops.end(), indexes.begin(), indexes.end());
  append_inst(functions, OpCompositeExtract, ops);
  return id;
}

Id Builder::emit_composite_construct(Id result_type,
                                     const std::vector<Id> &constituents) {
  auto id = make_id();
  std::vector<uint32_t> ops = {result_type, id};
  ops.insert(ops.end(), constituents.begin(), constituents.end());
  append_inst(functions, OpCompositeConstruct, ops);
  return id;
}

Id Builder::emit_composite_insert(Id result_type, Id object, Id composite,
                                  const std::vector<uint32_t> &indexes) {
  auto id = make_id();
  std::vector<uint32_t> ops = {result_type, id, object, composite};
  ops.insert(ops.end(), indexes.begin(), indexes.end());
  append_inst(functions, OpCompositeInsert, ops);
  return id;
}

Id Builder::emit_vector_shuffle(Id result_type, Id vec1, Id vec2,
                                const std::vector<uint32_t> &components) {
  auto id = make_id();
  std::vector<uint32_t> ops = {result_type, id, vec1, vec2};
  ops.insert(ops.end(), components.begin(), components.end());
  append_inst(functions, OpVectorShuffle, ops);
  return id;
}

Id Builder::emit_select(Id result_type, Id condition, Id true_val,
                        Id false_val) {
  auto id = make_id();
  append_inst(functions, OpSelect,
              {result_type, id, condition, true_val, false_val});
  return id;
}

Id Builder::emit_phi(Id result_type,
                     const std::vector<std::pair<Id, Id>> &incoming) {
  auto id = make_id();
  std::vector<uint32_t> ops = {result_type, id};
  for (auto &[value, block] : incoming) {
    ops.push_back(value);
    ops.push_back(block);
  }
  append_inst(functions, OpPhi, ops);
  return id;
}

Id Builder::emit_ext_inst(Id result_type, Id set, uint32_t instruction,
                          const std::vector<Id> &operands) {
  auto id = make_id();
  std::vector<uint32_t> ops = {result_type, id, set, instruction};
  ops.insert(ops.end(), operands.begin(), operands.end());
  append_inst(functions, OpExtInst, ops);
  return id;
}

Id Builder::emit_convert(Op opcode, Id result_type, Id value) {
  auto id = make_id();
  append_inst(functions, opcode, {result_type, id, value});
  return id;
}

Id Builder::emit_bitcast(Id result_type, Id value) {
  auto id = make_id();
  append_inst(functions, OpBitcast, {result_type, id, value});
  return id;
}

// ---- Image instructions ----

Id Builder::emit_sampled_image(Id result_type, Id image, Id sampler) {
  auto id = make_id();
  append_inst(functions, OpSampledImage, {result_type, id, image, sampler});
  return id;
}

Id Builder::emit_image_sample_implicit_lod(
    Id result_type, Id sampled_image, Id coordinate, uint32_t image_operands,
    const std::vector<Id> &operand_ids) {
  auto id = make_id();
  std::vector<uint32_t> ops = {result_type, id, sampled_image, coordinate};
  if (image_operands != 0) {
    ops.push_back(image_operands);
    ops.insert(ops.end(), operand_ids.begin(), operand_ids.end());
  }
  append_inst(functions, OpImageSampleImplicitLod, ops);
  return id;
}

Id Builder::emit_image_sample_explicit_lod(
    Id result_type, Id sampled_image, Id coordinate, uint32_t image_operands,
    const std::vector<Id> &operand_ids) {
  auto id = make_id();
  std::vector<uint32_t> ops = {result_type, id, sampled_image, coordinate};
  ops.push_back(image_operands);
  ops.insert(ops.end(), operand_ids.begin(), operand_ids.end());
  append_inst(functions, OpImageSampleExplicitLod, ops);
  return id;
}

Id Builder::emit_image_sample_dref_implicit_lod(
    Id result_type, Id sampled_image, Id coordinate, Id dref,
    uint32_t image_operands, const std::vector<Id> &operand_ids) {
  auto id = make_id();
  std::vector<uint32_t> ops = {result_type, id, sampled_image, coordinate,
                               dref};
  if (image_operands != 0) {
    ops.push_back(image_operands);
    ops.insert(ops.end(), operand_ids.begin(), operand_ids.end());
  }
  append_inst(functions, OpImageSampleDrefImplicitLod, ops);
  return id;
}

Id Builder::emit_image_fetch(Id result_type, Id image, Id coordinate,
                             uint32_t image_operands,
                             const std::vector<Id> &operand_ids) {
  auto id = make_id();
  std::vector<uint32_t> ops = {result_type, id, image, coordinate};
  if (image_operands != 0) {
    ops.push_back(image_operands);
    ops.insert(ops.end(), operand_ids.begin(), operand_ids.end());
  }
  append_inst(functions, OpImageFetch, ops);
  return id;
}

Id Builder::emit_image_read(Id result_type, Id image, Id coordinate) {
  auto id = make_id();
  append_inst(functions, OpImageRead, {result_type, id, image, coordinate});
  return id;
}

void Builder::emit_image_write(Id image, Id coordinate, Id texel) {
  append_inst(functions, OpImageWrite, {image, coordinate, texel});
}

Id Builder::emit_image(Id result_type, Id sampled_image) {
  auto id = make_id();
  append_inst(functions, OpImage, {result_type, id, sampled_image});
  return id;
}

Id Builder::emit_image_query_size_lod(Id result_type, Id image, Id lod) {
  auto id = make_id();
  append_inst(functions, OpImageQuerySizeLod, {result_type, id, image, lod});
  return id;
}

Id Builder::emit_image_query_size(Id result_type, Id image) {
  auto id = make_id();
  append_inst(functions, OpImageQuerySize, {result_type, id, image});
  return id;
}

Id Builder::emit_image_query_levels(Id result_type, Id image) {
  auto id = make_id();
  append_inst(functions, OpImageQueryLevels, {result_type, id, image});
  return id;
}

Id Builder::emit_image_query_samples(Id result_type, Id image) {
  auto id = make_id();
  append_inst(functions, OpImageQuerySamples, {result_type, id, image});
  return id;
}

// ---- Control flow ----

void Builder::emit_branch(Id target_label) {
  append_inst(functions, OpBranch, {target_label});
}

void Builder::emit_branch_conditional(Id condition, Id true_label,
                                      Id false_label,
                                      const std::vector<uint32_t> &weights) {
  std::vector<uint32_t> ops = {condition, true_label, false_label};
  ops.insert(ops.end(), weights.begin(), weights.end());
  append_inst(functions, OpBranchConditional, ops);
}

void Builder::emit_selection_merge(Id merge_block,
                                   SelectionControlMask control) {
  append_inst(functions, OpSelectionMerge,
              {merge_block, uint32_t(control)});
}

void Builder::emit_loop_merge(Id merge_block, Id continue_block,
                              LoopControlMask control) {
  append_inst(functions, OpLoopMerge,
              {merge_block, continue_block, uint32_t(control)});
}

void Builder::emit_switch(
    Id selector, Id default_label,
    const std::vector<std::pair<uint32_t, Id>> &cases) {
  std::vector<uint32_t> ops = {selector, default_label};
  for (auto &[literal, label] : cases) {
    ops.push_back(literal);
    ops.push_back(label);
  }
  append_inst(functions, OpSwitch, ops);
}

void Builder::emit_return() { append_inst(functions, OpReturn, {}); }

void Builder::emit_return_value(Id value) {
  append_inst(functions, OpReturnValue, {value});
}

void Builder::emit_kill() { append_inst(functions, OpKill, {}); }

void Builder::emit_unreachable() {
  append_inst(functions, OpUnreachable, {});
}

// ---- Function calls ----

Id Builder::emit_function_call(Id result_type, Id function,
                               const std::vector<Id> &args) {
  auto id = make_id();
  std::vector<uint32_t> ops = {result_type, id, function};
  ops.insert(ops.end(), args.begin(), args.end());
  append_inst(functions, OpFunctionCall, ops);
  return id;
}

// ---- Atomic / barrier ----

void Builder::emit_control_barrier(Id execution, Id memory, Id semantics) {
  append_inst(functions, OpControlBarrier, {execution, memory, semantics});
}

void Builder::emit_memory_barrier(Id memory, Id semantics) {
  append_inst(functions, OpMemoryBarrier, {memory, semantics});
}

// ---- Finalization ----

void Builder::fail(const std::string &message) {
  if (error.empty()) {
    error = message;
  }
}

void Builder::finalize() {
  if (module_finalized || has_error()) {
    return;
  }

  blob.clear();
  size_t total = 5 + capabilities.size() + extensions.size() +
                 ext_imports.size() + memory_model_section.size() +
                 entry_points.size() + execution_modes.size() + debug.size() +
                 annotations.size() + types_globals.size() + functions.size();
  blob.reserve(total);

  blob.push_back(MagicNumber);
  blob.push_back(get_version_word(options.vk_version));
  blob.push_back(0x000D0001u);
  blob.push_back(next_id);
  blob.push_back(0);

  auto emit = [&](const std::vector<uint32_t> &section) {
    blob.insert(blob.end(), section.begin(), section.end());
  };

  emit(capabilities);
  emit(extensions);
  emit(ext_imports);
  emit(memory_model_section);
  emit(entry_points);
  emit(execution_modes);
  emit(debug);
  emit(annotations);
  emit(types_globals);
  emit(functions);

  module_finalized = true;
}

} // namespace spv
