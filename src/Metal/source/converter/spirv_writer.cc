#include "metalib2spirv_p.h"
#include "spirv_builder.h"
#include "spirv/unified1/GLSL.std.450.h"

#include <array>
#include <cstring>

namespace spv {
using namespace llvm;

// Maps LLVM IR values/types to SPIR-V IDs and drives the lowering.
class Writer {
public:
  Writer(const llvm::Module &mod, Builder &builder);

  llvm::Error generate();

private:
  // ---- Module analysis ----
  const Function *get_entry_function() const;
  MDNode *get_entry_metadata() const;
  bool has_entry_option(StringRef option_name) const;
  std::array<uint32_t, 3> get_local_size() const;
  static ExecutionModel get_execution_model(mtl2spv_shader_type shader_type);

  // ---- Type conversion ----
  Id convert_type(llvm::Type *type);
  Id convert_struct_type(llvm::StructType *st);
  Id get_scalar_type(llvm::Type *type);

  // ---- Interface variable creation from AIR metadata ----
  void process_entry_arguments();
  void create_buffer_variable(MDNode *arg_node, unsigned func_arg_idx);
  void create_texture_variable(MDNode *arg_node, unsigned func_arg_idx);
  void create_sampler_variable(MDNode *arg_node, unsigned func_arg_idx);
  void create_stage_in_struct(MDNode *arg_list_node, unsigned func_arg_idx);
  void create_builtin_input(StringRef builtin_name, llvm::Argument *arg);
  void create_output_variables();
  void create_vertex_output(MDNode *output_md);
  void create_fragment_output(MDNode *output_md);

  // ---- Instruction lowering ----
  void lower_function_body(const Function &func);
  Id lower_value(const Value *val);
  Id lower_constant(const Constant *c);
  void lower_instruction(const Instruction &inst);
  void lower_alloca(const AllocaInst &inst);
  void lower_load(const LoadInst &inst);
  void lower_store(const StoreInst &inst);
  void lower_gep(const GetElementPtrInst &inst);
  void lower_binary(const BinaryOperator &inst);
  void lower_icmp(const ICmpInst &inst);
  void lower_fcmp(const FCmpInst &inst);
  void lower_cast(const CastInst &inst);
  void lower_select(const SelectInst &inst);
  void lower_phi(const PHINode &inst);
  void lower_extractelement(const ExtractElementInst &inst);
  void lower_insertelement(const InsertElementInst &inst);
  void lower_shufflevector(const ShuffleVectorInst &inst);
  void lower_extractvalue(const ExtractValueInst &inst);
  void lower_insertvalue(const InsertValueInst &inst);
  void lower_return(const ReturnInst &inst);
  void lower_branch(const BranchInst &inst);
  void lower_switch(const SwitchInst &inst);
  void lower_call(const CallInst &inst);

  // ---- AIR intrinsic lowering ----
  Id lower_air_intrinsic(const CallInst &call, StringRef callee_name);
  Id lower_air_sample(const CallInst &call, StringRef name);
  Id lower_air_read_texture(const CallInst &call, StringRef name);
  Id lower_air_write_texture(const CallInst &call, StringRef name);
  Id lower_air_math(const CallInst &call, StringRef name);
  Id lower_air_barrier(const CallInst &call, StringRef name);
  Id lower_llvm_intrinsic(const CallInst &call, StringRef name);

  const llvm::Module &mod;
  Builder &builder;

  std::unordered_map<const Value *, Id> value_map;
  std::unordered_map<llvm::Type *, Id> type_map;
  std::unordered_map<const BasicBlock *, Id> block_map;

  std::vector<Id> interface_ids;

  // Texture/sampler argument maps: arg index -> spv variable id
  std::unordered_map<unsigned, Id> texture_vars;
  std::unordered_map<unsigned, Id> sampler_vars;
  std::unordered_map<unsigned, Id> buffer_vars;
  // Maps AIR texture arg index -> image type id
  std::unordered_map<unsigned, Id> texture_image_types;

  // Output variable (for vertex: struct output, for fragment: per-attachment)
  std::vector<std::pair<unsigned, Id>> output_vars;
  Id output_position_var = 0;
  Id output_point_size_var = 0;

  Id glsl_ext_inst = 0;
  Id entry_function_id = 0;

  unsigned next_binding = 0;
  unsigned next_input_location = 0;
  unsigned next_output_location = 0;
};

Writer::Writer(const llvm::Module &in_mod, Builder &in_builder)
    : mod(in_mod), builder(in_builder) {}

const Function *Writer::get_entry_function() const {
  return mod.getFunction(builder.entry_name.c_str());
}

MDNode *Writer::get_entry_metadata() const {
  const char *metadata_name = nullptr;
  switch (builder.shader_type) {
  case MTL2SPV_SHADER_VERTEX:
    metadata_name = "air.vertex";
    break;
  case MTL2SPV_SHADER_FRAGMENT:
    metadata_name = "air.fragment";
    break;
  case MTL2SPV_SHADER_COMPUTE:
    metadata_name = "air.kernel";
    break;
  default:
    return nullptr;
  }

  auto *named = mod.getNamedMetadata(metadata_name);
  if (!named || named->getNumOperands() == 0)
    return nullptr;

  for (unsigned i = 0; i < named->getNumOperands(); ++i) {
    auto *op = named->getOperand(i);
    if (!op || op->getNumOperands() < 1)
      continue;
    auto *func_md = dyn_cast_or_null<ValueAsMetadata>(op->getOperand(0));
    if (!func_md)
      continue;
    if (auto *f = dyn_cast<Function>(func_md->getValue())) {
      if (f->getName() == builder.entry_name.c_str())
        return op;
    }
  }

  return named->getOperand(0);
}

bool Writer::has_entry_option(StringRef option_name) const {
  auto *entry_md = get_entry_metadata();
  if (!entry_md)
    return false;

  for (unsigned i = 3; i < entry_md->getNumOperands(); ++i) {
    if (const auto *str = dyn_cast<MDString>(entry_md->getOperand(i))) {
      if (str->getString() == option_name)
        return true;
    }
    if (auto *sub = dyn_cast<MDNode>(entry_md->getOperand(i))) {
      if (sub->getNumOperands() > 0) {
        if (auto *s = dyn_cast<MDString>(sub->getOperand(0))) {
          if (s->getString() == option_name)
            return true;
        }
      }
    }
  }

  return false;
}

std::array<uint32_t, 3> Writer::get_local_size() const {
  const auto *entry = get_entry_function();
  if (!entry)
    return {1, 1, 1};

  if (auto *local_size = entry->getMetadata("reqd_work_group_size")) {
    if (local_size->getNumOperands() >= 3) {
      return {
          uint32_t(cast_operand_to_int64(local_size, 0)),
          uint32_t(cast_operand_to_int64(local_size, 1)),
          uint32_t(cast_operand_to_int64(local_size, 2)),
      };
    }
  }

  return {1, 1, 1};
}

ExecutionModel Writer::get_execution_model(mtl2spv_shader_type shader_type) {
  switch (shader_type) {
  case MTL2SPV_SHADER_VERTEX:
    return ExecutionModelVertex;
  case MTL2SPV_SHADER_FRAGMENT:
    return ExecutionModelFragment;
  case MTL2SPV_SHADER_COMPUTE:
    return ExecutionModelGLCompute;
  default:
    return ExecutionModelMax;
  }
}

// ---- Type conversion ----

Id Writer::convert_type(llvm::Type *type) {
  auto it = type_map.find(type);
  if (it != type_map.end())
    return it->second;

  Id result = 0;

  if (type->isVoidTy()) {
    result = builder.void_type();
  } else if (type->isIntegerTy(1)) {
    result = builder.bool_type();
  } else if (type->isIntegerTy()) {
    unsigned width = type->getIntegerBitWidth();
    if (width < 16)
      width = 32;
    result = builder.int_type(width, true);
  } else if (type->isHalfTy()) {
    result = builder.float_type(16);
  } else if (type->isFloatTy()) {
    result = builder.float_type(32);
  } else if (type->isDoubleTy()) {
    result = builder.float_type(64);
  } else if (auto *vt = dyn_cast<FixedVectorType>(type)) {
    auto elem_id = convert_type(vt->getElementType());
    result = builder.vector_type(elem_id, vt->getNumElements());
  } else if (auto *at = dyn_cast<ArrayType>(type)) {
    auto elem_id = convert_type(at->getElementType());
    auto len_id = builder.uint_constant(builder.int_type(32, false),
                                        at->getNumElements());
    result = builder.array_type(elem_id, len_id);
    auto stride = mod.getDataLayout().getTypeAllocSize(at->getElementType());
    builder.decorate(result, DecorationArrayStride, {uint32_t(stride)});
  } else if (auto *st = dyn_cast<StructType>(type)) {
    result = convert_struct_type(st);
  } else if (auto *pt = dyn_cast<PointerType>(type)) {
    // For opaque pointers, default to 32-bit int pointee
    auto sc = air_address_space_to_spirv(pt->getAddressSpace());
    auto pointee = builder.int_type(32, true);
    result = builder.pointer_type(pointee, sc);
  } else {
    result = builder.float_type(32);
  }

  type_map[type] = result;
  return result;
}

Id Writer::convert_struct_type(llvm::StructType *st) {
  std::vector<Id> member_types;
  for (unsigned i = 0; i < st->getNumElements(); ++i) {
    member_types.push_back(convert_type(st->getElementType(i)));
  }
  auto id = builder.struct_type(member_types);

  if (st->hasName()) {
    builder.add_name(id, st->getName().str().c_str());
  }

  const auto &dl = mod.getDataLayout();
  auto *layout = dl.getStructLayout(st);
  for (unsigned i = 0; i < st->getNumElements(); ++i) {
    builder.member_decorate(id, i, DecorationOffset,
                            {uint32_t(layout->getElementOffset(i))});
  }

  return id;
}

Id Writer::get_scalar_type(llvm::Type *type) {
  if (auto *vt = dyn_cast<FixedVectorType>(type))
    return convert_type(vt->getElementType());
  return convert_type(type);
}

// ---- Interface variable creation ----

void Writer::process_entry_arguments() {
  auto *entry_md = get_entry_metadata();
  if (!entry_md || entry_md->getNumOperands() < 3)
    return;

  auto *arg_list = dyn_cast_or_null<MDNode>(entry_md->getOperand(2));
  if (!arg_list)
    return;

  const auto *entry_func = get_entry_function();
  if (!entry_func)
    return;

  for (unsigned ai = 0; ai < arg_list->getNumOperands(); ++ai) {
    auto *arg_parent = dyn_cast_or_null<MDNode>(arg_list->getOperand(ai));
    if (!arg_parent || arg_parent->getNumOperands() < 2)
      continue;

    auto *storage_md =
        dyn_cast_or_null<MDString>(arg_parent->getOperand(1));
    if (!storage_md)
      continue;

    auto storage_type = storage_md->getString();
    unsigned func_arg_idx = 0;
    if (auto *ci = mdconst::dyn_extract<ConstantInt>(arg_parent->getOperand(0)))
      func_arg_idx = (unsigned)ci->getZExtValue();

    if (storage_type == "air.buffer" ||
        storage_type == "air.indirect_buffer") {
      create_buffer_variable(arg_parent, func_arg_idx);
    } else if (storage_type == "air.texture") {
      create_texture_variable(arg_parent, func_arg_idx);
    } else if (storage_type == "air.sampler") {
      create_sampler_variable(arg_parent, func_arg_idx);
    } else if (storage_type == "air.stage_in") {
      create_stage_in_struct(arg_parent, func_arg_idx);
    } else if (StringRef(storage_type).startswith("air.")) {
      // Builtin inputs (e.g. air.vertex_id, air.thread_position_in_grid)
      if (func_arg_idx < entry_func->arg_size()) {
        auto *arg = const_cast<Argument *>(entry_func->getArg(func_arg_idx));
        create_builtin_input(storage_type, arg);
      }
    }
  }
}

void Writer::create_buffer_variable(MDNode *arg_node, unsigned func_arg_idx) {
  unsigned binding_index = 0;
  if (arg_node->getNumOperands() > 5) {
    if (auto *ci = mdconst::dyn_extract<ConstantInt>(arg_node->getOperand(5)))
      binding_index = (unsigned)ci->getZExtValue();
    else if (arg_node->getNumOperands() > 3) {
      if (auto *ci2 =
              mdconst::dyn_extract<ConstantInt>(arg_node->getOperand(3)))
        binding_index = (unsigned)ci2->getZExtValue();
    }
  }

  const auto *entry_func = get_entry_function();
  if (!entry_func || func_arg_idx >= entry_func->arg_size())
    return;

  auto *arg = entry_func->getArg(func_arg_idx);
  auto *arg_type = arg->getType();

  Id element_type;
  if (auto *pt = dyn_cast<PointerType>(arg_type)) {
    auto pointee = pt->getPointerElementType();
    if (auto *st = dyn_cast<StructType>(pointee)) {
      element_type = convert_struct_type(st);
      builder.decorate(element_type, DecorationBlock);
    } else {
      auto inner = convert_type(pointee);
      auto rt_array = builder.runtime_array_type(inner);
      auto stride = mod.getDataLayout().getTypeAllocSize(pointee);
      builder.decorate(rt_array, DecorationArrayStride, {uint32_t(stride)});
      element_type = builder.struct_type({rt_array});
      builder.decorate(element_type, DecorationBlock);
      builder.member_decorate(element_type, 0, DecorationOffset, {0});
    }
  } else {
    element_type = convert_type(arg_type);
  }

  bool is_readonly = true;
  if (arg_node->getNumOperands() > 7) {
    auto access_str = cast_operand_to_string(arg_node, 7);
    if (access_str == "air.write" || access_str == "air.read_write")
      is_readonly = false;
  }

  auto sc = is_readonly ? StorageClassUniform : StorageClassStorageBuffer;
  auto ptr_type = builder.pointer_type(element_type, sc);
  auto var_id = builder.global_variable(ptr_type, sc);

  builder.decorate(var_id, DecorationDescriptorSet, {0});
  builder.decorate(var_id, DecorationBinding, {binding_index});

  if (is_readonly)
    builder.decorate(var_id, DecorationNonWritable);

  buffer_vars[func_arg_idx] = var_id;
  value_map[arg] = var_id;
  interface_ids.push_back(var_id);
}

void Writer::create_texture_variable(MDNode *arg_node, unsigned func_arg_idx) {
  unsigned binding_index = 0;
  if (arg_node->getNumOperands() > 3) {
    if (auto *ci = mdconst::dyn_extract<ConstantInt>(arg_node->getOperand(3)))
      binding_index = (unsigned)ci->getZExtValue();
  }

  Id sampled_component_type = builder.float_type(32);
  std::string type_name;
  if (arg_node->getNumOperands() > 7) {
    if (auto *type_md = dyn_cast_or_null<MDString>(arg_node->getOperand(7)))
      type_name = type_md->getString().str();
  }

  auto open = type_name.find('<');
  if (open != std::string::npos) {
    auto end = type_name.find_first_of(",>", open + 1);
    if (end != std::string::npos) {
      auto elem = type_name.substr(open + 1, end - open - 1);
      if (elem == "half")
        sampled_component_type = builder.float_type(16);
      else if (elem == "int" || elem == "int32_t")
        sampled_component_type = builder.int_type(32, true);
      else if (elem == "uint" || elem == "uint32_t")
        sampled_component_type = builder.int_type(32, false);
      else if (elem == "short" || elem == "int16_t")
        sampled_component_type = builder.int_type(16, true);
      else if (elem == "ushort" || elem == "uint16_t")
        sampled_component_type = builder.int_type(16, false);
    }
  }

  auto dim = air_texture_dim(type_name);
  bool arrayed = air_texture_arrayed(type_name);
  bool ms = air_texture_ms(type_name);
  bool is_depth = air_is_depth_texture(type_name);

  bool is_write_only = false;
  if (arg_node->getNumOperands() > 5) {
    auto access_str = cast_operand_to_string(arg_node, 5);
    is_write_only = (access_str == "air.write");
  }

  uint32_t sampled_flag = is_write_only ? 2 : 1;
  auto img_type = builder.image_type(sampled_component_type, dim,
                                     is_depth ? 1 : 0, arrayed ? 1 : 0,
                                     ms ? 1 : 0, sampled_flag,
                                     ImageFormatUnknown);

  texture_image_types[func_arg_idx] = img_type;

  auto ptr_type =
      builder.pointer_type(img_type, StorageClassUniformConstant);
  auto var_id = builder.global_variable(ptr_type, StorageClassUniformConstant);

  builder.decorate(var_id, DecorationDescriptorSet, {0});
  builder.decorate(var_id, DecorationBinding, {binding_index});

  const auto *entry_func = get_entry_function();
  if (entry_func && func_arg_idx < entry_func->arg_size()) {
    value_map[entry_func->getArg(func_arg_idx)] = var_id;
  }

  texture_vars[func_arg_idx] = var_id;
  interface_ids.push_back(var_id);
}

void Writer::create_sampler_variable(MDNode *arg_node, unsigned func_arg_idx) {
  unsigned binding_index = 0;
  if (arg_node->getNumOperands() > 3) {
    if (auto *ci = mdconst::dyn_extract<ConstantInt>(arg_node->getOperand(3)))
      binding_index = (unsigned)ci->getZExtValue();
  }

  auto samp_type = builder.sampler_type();
  auto ptr_type =
      builder.pointer_type(samp_type, StorageClassUniformConstant);
  auto var_id = builder.global_variable(ptr_type, StorageClassUniformConstant);

  builder.decorate(var_id, DecorationDescriptorSet, {1});
  builder.decorate(var_id, DecorationBinding, {binding_index});

  const auto *entry_func = get_entry_function();
  if (entry_func && func_arg_idx < entry_func->arg_size()) {
    value_map[entry_func->getArg(func_arg_idx)] = var_id;
  }

  sampler_vars[func_arg_idx] = var_id;
  interface_ids.push_back(var_id);
}

void Writer::create_stage_in_struct(MDNode *arg_node, unsigned func_arg_idx) {
  for (unsigned i = 2; i < arg_node->getNumOperands(); ++i) {
    auto *attr_md = dyn_cast_or_null<MDNode>(arg_node->getOperand(i));
    if (!attr_md || attr_md->getNumOperands() < 3)
      continue;

    auto tag = cast_operand_to_string(attr_md, 0);
    if (tag != "air.location_index")
      continue;

    unsigned location = (unsigned)cast_operand_to_int64(attr_md, 1);

    llvm::Type *attr_type = nullptr;
    for (unsigned j = 3; j < attr_md->getNumOperands(); ++j) {
      if (auto *s = dyn_cast_or_null<MDString>(attr_md->getOperand(j))) {
        if (s->getString() == "air.arg_type_name" &&
            j + 1 < attr_md->getNumOperands()) {
          auto type_str =
              cast_operand_to_string(attr_md, j + 1);
          if (type_str == "float4")
            attr_type = FixedVectorType::get(
                Type::getFloatTy(mod.getContext()), 4);
          else if (type_str == "float3")
            attr_type = FixedVectorType::get(
                Type::getFloatTy(mod.getContext()), 3);
          else if (type_str == "float2")
            attr_type = FixedVectorType::get(
                Type::getFloatTy(mod.getContext()), 2);
          else if (type_str == "float")
            attr_type = Type::getFloatTy(mod.getContext());
          else if (type_str == "uint" || type_str == "int")
            attr_type = Type::getInt32Ty(mod.getContext());
          else if (type_str == "half4")
            attr_type = FixedVectorType::get(
                Type::getHalfTy(mod.getContext()), 4);
          else
            attr_type = FixedVectorType::get(
                Type::getFloatTy(mod.getContext()), 4);
          break;
        }
      }
    }

    if (!attr_type)
      attr_type = FixedVectorType::get(Type::getFloatTy(mod.getContext()), 4);

    auto spv_type = convert_type(attr_type);
    auto ptr_type = builder.pointer_type(spv_type, StorageClassInput);
    auto var_id = builder.global_variable(ptr_type, StorageClassInput);
    builder.decorate(var_id, DecorationLocation, {location});
    interface_ids.push_back(var_id);
  }
}

void Writer::create_builtin_input(StringRef builtin_name, Argument *arg) {
  auto builtin_enum = air_builtin_to_spirv(builtin_name, builder.shader_type);
  if (builtin_enum == BuiltInMax)
    return;

  auto *arg_type = arg->getType();
  auto spv_type = convert_type(arg_type);

  if (builtin_enum == BuiltInSampleMask) {
    auto u32_ty = builder.int_type(32, false);
    auto one_id = builder.uint_constant(builder.int_type(32, false), 1);
    spv_type = builder.array_type(u32_ty, one_id);
  }

  auto ptr_type = builder.pointer_type(spv_type, StorageClassInput);
  auto var_id = builder.global_variable(ptr_type, StorageClassInput);
  builder.decorate(var_id, DecorationBuiltIn, {uint32_t(builtin_enum)});

  value_map[arg] = var_id;
  interface_ids.push_back(var_id);
}

void Writer::create_output_variables() {
  auto *entry_md = get_entry_metadata();
  if (!entry_md)
    return;

  if (entry_md->getNumOperands() > 1) {
    auto *output_md = dyn_cast_or_null<MDNode>(entry_md->getOperand(1));
    if (output_md) {
      if (builder.shader_type == MTL2SPV_SHADER_VERTEX)
        create_vertex_output(output_md);
      else if (builder.shader_type == MTL2SPV_SHADER_FRAGMENT)
        create_fragment_output(output_md);
    }
  }

  if (builder.shader_type == MTL2SPV_SHADER_VERTEX) {
    if (output_position_var == 0) {
      auto f32_ty = builder.float_type(32);
      auto v4f32_ty = builder.vector_type(f32_ty, 4);
      auto ptr_type = builder.pointer_type(v4f32_ty, StorageClassOutput);
      output_position_var =
          builder.global_variable(ptr_type, StorageClassOutput);
      builder.decorate(output_position_var, DecorationBuiltIn,
                       {uint32_t(BuiltInPosition)});
      interface_ids.push_back(output_position_var);
    }
  }
}

void Writer::create_vertex_output(MDNode *output_md) {
  for (unsigned i = 0; i < output_md->getNumOperands(); ++i) {
    auto *attr = dyn_cast_or_null<MDNode>(output_md->getOperand(i));
    if (!attr || attr->getNumOperands() < 1)
      continue;

    auto tag = cast_operand_to_string(attr, 0);
    if (tag == "air.position") {
      auto f32_ty = builder.float_type(32);
      auto v4f32_ty = builder.vector_type(f32_ty, 4);
      auto ptr_type = builder.pointer_type(v4f32_ty, StorageClassOutput);
      output_position_var =
          builder.global_variable(ptr_type, StorageClassOutput);
      builder.decorate(output_position_var, DecorationBuiltIn,
                       {uint32_t(BuiltInPosition)});
      interface_ids.push_back(output_position_var);
    } else if (tag == "air.point_size") {
      auto f32_ty = builder.float_type(32);
      auto ptr_type = builder.pointer_type(f32_ty, StorageClassOutput);
      output_point_size_var =
          builder.global_variable(ptr_type, StorageClassOutput);
      builder.decorate(output_point_size_var, DecorationBuiltIn,
                       {uint32_t(BuiltInPointSize)});
      interface_ids.push_back(output_point_size_var);
    } else if (tag == "air.location_index" ||
               StringRef(tag).startswith("air.vertex_output") ||
               StringRef(tag).startswith("air.visible_output")) {
      unsigned location = next_output_location++;
      if (attr->getNumOperands() > 1) {
        if (auto *ci = mdconst::dyn_extract<ConstantInt>(attr->getOperand(1)))
          location = (unsigned)ci->getZExtValue();
      }
      auto f32_ty = builder.float_type(32);
      auto v4f32_ty = builder.vector_type(f32_ty, 4);
      auto ptr_type = builder.pointer_type(v4f32_ty, StorageClassOutput);
      auto var_id = builder.global_variable(ptr_type, StorageClassOutput);
      builder.decorate(var_id, DecorationLocation, {location});
      output_vars.push_back({i, var_id});
      interface_ids.push_back(var_id);
    }
  }
}

void Writer::create_fragment_output(MDNode *output_md) {
  for (unsigned i = 0; i < output_md->getNumOperands(); ++i) {
    auto *attr = dyn_cast_or_null<MDNode>(output_md->getOperand(i));
    if (!attr || attr->getNumOperands() < 1)
      continue;

    auto tag = cast_operand_to_string(attr, 0);

    if (tag == "air.render_target" || tag == "air.color") {
      unsigned location = i;
      if (attr->getNumOperands() > 1) {
        if (auto *ci = mdconst::dyn_extract<ConstantInt>(attr->getOperand(1)))
          location = (unsigned)ci->getZExtValue();
      }
      auto f32_ty = builder.float_type(32);
      auto v4f32_ty = builder.vector_type(f32_ty, 4);
      auto ptr_type = builder.pointer_type(v4f32_ty, StorageClassOutput);
      auto var_id = builder.global_variable(ptr_type, StorageClassOutput);
      builder.decorate(var_id, DecorationLocation, {location});
      output_vars.push_back({i, var_id});
      interface_ids.push_back(var_id);
    } else if (tag == "air.depth") {
      auto f32_ty = builder.float_type(32);
      auto ptr_type = builder.pointer_type(f32_ty, StorageClassOutput);
      auto var_id = builder.global_variable(ptr_type, StorageClassOutput);
      builder.decorate(var_id, DecorationBuiltIn,
                       {uint32_t(BuiltInFragDepth)});
      output_vars.push_back({i, var_id});
      interface_ids.push_back(var_id);
    }
  }
}

// ---- Value lowering ----

Id Writer::lower_value(const Value *val) {
  if (!val)
    return 0;

  auto it = value_map.find(val);
  if (it != value_map.end())
    return it->second;

  if (auto *c = dyn_cast<Constant>(val))
    return lower_constant(c);

  builder.fail("unmapped value encountered");
  return 0;
}

Id Writer::lower_constant(const Constant *c) {
  auto it = value_map.find(c);
  if (it != value_map.end())
    return it->second;

  Id result = 0;
  auto *type = c->getType();

  if (isa<UndefValue>(c)) {
    auto spv_type = convert_type(type);
    result = builder.null_constant(spv_type);
  } else if (auto *ci = dyn_cast<ConstantInt>(c)) {
    auto spv_type = convert_type(type);
    auto bw = ci->getBitWidth();
    if (bw == 1) {
      result = builder.bool_constant(ci->isOne());
    } else if (bw <= 32) {
      int32_t val = (int32_t)ci->getSExtValue();
      result = builder.int_constant(spv_type, val);
    } else {
      result = builder.int64_constant(spv_type, ci->getSExtValue());
    }
  } else if (auto *cf = dyn_cast<ConstantFP>(c)) {
    auto spv_type = convert_type(type);
    if (type->isHalfTy() || type->isFloatTy()) {
      result = builder.float_constant(spv_type, cf->getValueAPF().convertToFloat());
    } else {
      result = builder.double_constant(spv_type, cf->getValueAPF().convertToDouble());
    }
  } else if (isa<ConstantAggregateZero>(c)) {
    result = builder.null_constant(convert_type(type));
  } else if (auto *ca = dyn_cast<ConstantArray>(c)) {
    auto spv_type = convert_type(type);
    std::vector<Id> elems;
    for (unsigned i = 0; i < ca->getNumOperands(); ++i)
      elems.push_back(lower_constant(ca->getOperand(i)));
    result = builder.composite_constant(spv_type, elems);
  } else if (auto *cv = dyn_cast<ConstantVector>(c)) {
    auto spv_type = convert_type(type);
    std::vector<Id> elems;
    for (unsigned i = 0; i < cv->getNumOperands(); ++i)
      elems.push_back(lower_constant(cv->getOperand(i)));
    result = builder.composite_constant(spv_type, elems);
  } else if (auto *cs = dyn_cast<ConstantStruct>(c)) {
    auto spv_type = convert_type(type);
    std::vector<Id> elems;
    for (unsigned i = 0; i < cs->getNumOperands(); ++i)
      elems.push_back(lower_constant(cs->getOperand(i)));
    result = builder.composite_constant(spv_type, elems);
  } else if (auto *cdv = dyn_cast<ConstantDataVector>(c)) {
    auto spv_type = convert_type(type);
    std::vector<Id> elems;
    for (unsigned i = 0; i < cdv->getNumElements(); ++i)
      elems.push_back(lower_constant(cdv->getElementAsConstant(i)));
    result = builder.composite_constant(spv_type, elems);
  } else if (isa<ConstantPointerNull>(c)) {
    result = builder.null_constant(convert_type(type));
  } else if (auto *ce = dyn_cast<ConstantExpr>(c)) {
    // For constant expressions, lower the equivalent instruction
    auto *inst = ce->getAsInstruction();
    lower_instruction(*inst);
    result = value_map[inst];
    value_map[c] = result;
    inst->deleteValue();
    return result;
  } else {
    result = builder.null_constant(convert_type(type));
  }

  value_map[c] = result;
  return result;
}

// ---- Instruction lowering ----

void Writer::lower_instruction(const Instruction &inst) {
  if (isa<AllocaInst>(inst))
    lower_alloca(cast<AllocaInst>(inst));
  else if (isa<LoadInst>(inst))
    lower_load(cast<LoadInst>(inst));
  else if (isa<StoreInst>(inst))
    lower_store(cast<StoreInst>(inst));
  else if (isa<GetElementPtrInst>(inst))
    lower_gep(cast<GetElementPtrInst>(inst));
  else if (isa<BinaryOperator>(inst))
    lower_binary(cast<BinaryOperator>(inst));
  else if (isa<ICmpInst>(inst))
    lower_icmp(cast<ICmpInst>(inst));
  else if (isa<FCmpInst>(inst))
    lower_fcmp(cast<FCmpInst>(inst));
  else if (isa<CastInst>(inst))
    lower_cast(cast<CastInst>(inst));
  else if (isa<SelectInst>(inst))
    lower_select(cast<SelectInst>(inst));
  else if (isa<PHINode>(inst))
    lower_phi(cast<PHINode>(inst));
  else if (isa<ExtractElementInst>(inst))
    lower_extractelement(cast<ExtractElementInst>(inst));
  else if (isa<InsertElementInst>(inst))
    lower_insertelement(cast<InsertElementInst>(inst));
  else if (isa<ShuffleVectorInst>(inst))
    lower_shufflevector(cast<ShuffleVectorInst>(inst));
  else if (isa<ExtractValueInst>(inst))
    lower_extractvalue(cast<ExtractValueInst>(inst));
  else if (isa<InsertValueInst>(inst))
    lower_insertvalue(cast<InsertValueInst>(inst));
  else if (isa<ReturnInst>(inst))
    lower_return(cast<ReturnInst>(inst));
  else if (isa<BranchInst>(inst))
    lower_branch(cast<BranchInst>(inst));
  else if (isa<SwitchInst>(inst))
    lower_switch(cast<SwitchInst>(inst));
  else if (isa<CallInst>(inst))
    lower_call(cast<CallInst>(inst));
  else if (isa<UnreachableInst>(inst))
    builder.emit_unreachable();
  // Ignore instructions we can't handle yet
}

void Writer::lower_alloca(const AllocaInst &inst) {
  auto *alloc_type = inst.getAllocatedType();
  auto spv_type = convert_type(alloc_type);
  auto ptr_type = builder.pointer_type(spv_type, StorageClassFunction);
  auto var_id = builder.function_variable(ptr_type);
  value_map[&inst] = var_id;
}

void Writer::lower_load(const LoadInst &inst) {
  auto ptr_id = lower_value(inst.getPointerOperand());
  auto spv_type = convert_type(inst.getType());
  auto result = builder.emit_load(spv_type, ptr_id);
  value_map[&inst] = result;
}

void Writer::lower_store(const StoreInst &inst) {
  auto val_id = lower_value(inst.getValueOperand());
  auto ptr_id = lower_value(inst.getPointerOperand());
  builder.emit_store(ptr_id, val_id);
}

void Writer::lower_gep(const GetElementPtrInst &inst) {
  auto base_id = lower_value(inst.getPointerOperand());
  std::vector<Id> idx;
  for (unsigned i = 1; i < inst.getNumOperands(); ++i) {
    idx.push_back(lower_value(inst.getOperand(i)));
  }

  auto *result_pointee = inst.getResultElementType();
  auto pointee_type = convert_type(result_pointee);

  auto *ptr_type_llvm = inst.getType();
  auto sc = StorageClassFunction;
  if (auto *pt = dyn_cast<PointerType>(ptr_type_llvm))
    sc = air_address_space_to_spirv(pt->getAddressSpace());

  auto result_ptr_type = builder.pointer_type(pointee_type, sc);
  auto result = builder.emit_access_chain(result_ptr_type, base_id, idx);
  value_map[&inst] = result;
}

void Writer::lower_binary(const BinaryOperator &inst) {
  auto lhs = lower_value(inst.getOperand(0));
  auto rhs = lower_value(inst.getOperand(1));
  auto result_type = convert_type(inst.getType());

  auto *type = inst.getType();
  bool is_float = type->isFPOrFPVectorTy();
  bool is_signed = true;

  Op opcode;
  switch (inst.getOpcode()) {
  case Instruction::Add:
  case Instruction::FAdd:
    opcode = is_float ? OpFAdd : OpIAdd;
    break;
  case Instruction::Sub:
  case Instruction::FSub:
    opcode = is_float ? OpFSub : OpISub;
    break;
  case Instruction::Mul:
  case Instruction::FMul:
    opcode = is_float ? OpFMul : OpIMul;
    break;
  case Instruction::UDiv:
    opcode = OpUDiv;
    break;
  case Instruction::SDiv:
    opcode = OpSDiv;
    break;
  case Instruction::FDiv:
    opcode = OpFDiv;
    break;
  case Instruction::URem:
    opcode = OpUMod;
    break;
  case Instruction::SRem:
    opcode = OpSRem;
    break;
  case Instruction::FRem:
    opcode = OpFRem;
    break;
  case Instruction::Shl:
    opcode = OpShiftLeftLogical;
    break;
  case Instruction::LShr:
    opcode = OpShiftRightLogical;
    break;
  case Instruction::AShr:
    opcode = OpShiftRightArithmetic;
    break;
  case Instruction::And:
    opcode = OpBitwiseAnd;
    break;
  case Instruction::Or:
    opcode = OpBitwiseOr;
    break;
  case Instruction::Xor:
    opcode = OpBitwiseXor;
    break;
  default:
    opcode = is_float ? OpFAdd : OpIAdd;
    break;
  }

  auto result = builder.emit_binop(opcode, result_type, lhs, rhs);
  value_map[&inst] = result;
}

void Writer::lower_icmp(const ICmpInst &inst) {
  auto lhs = lower_value(inst.getOperand(0));
  auto rhs = lower_value(inst.getOperand(1));
  auto result_type = convert_type(inst.getType());

  Op opcode;
  switch (inst.getPredicate()) {
  case ICmpInst::ICMP_EQ:
    opcode = OpIEqual;
    break;
  case ICmpInst::ICMP_NE:
    opcode = OpINotEqual;
    break;
  case ICmpInst::ICMP_UGT:
    opcode = OpUGreaterThan;
    break;
  case ICmpInst::ICMP_UGE:
    opcode = OpUGreaterThanEqual;
    break;
  case ICmpInst::ICMP_ULT:
    opcode = OpULessThan;
    break;
  case ICmpInst::ICMP_ULE:
    opcode = OpULessThanEqual;
    break;
  case ICmpInst::ICMP_SGT:
    opcode = OpSGreaterThan;
    break;
  case ICmpInst::ICMP_SGE:
    opcode = OpSGreaterThanEqual;
    break;
  case ICmpInst::ICMP_SLT:
    opcode = OpSLessThan;
    break;
  case ICmpInst::ICMP_SLE:
    opcode = OpSLessThanEqual;
    break;
  default:
    opcode = OpIEqual;
    break;
  }

  value_map[&inst] = builder.emit_binop(opcode, result_type, lhs, rhs);
}

void Writer::lower_fcmp(const FCmpInst &inst) {
  auto lhs = lower_value(inst.getOperand(0));
  auto rhs = lower_value(inst.getOperand(1));
  auto result_type = convert_type(inst.getType());

  Op opcode;
  switch (inst.getPredicate()) {
  case FCmpInst::FCMP_OEQ:
    opcode = OpFOrdEqual;
    break;
  case FCmpInst::FCMP_ONE:
    opcode = OpFOrdNotEqual;
    break;
  case FCmpInst::FCMP_OGT:
    opcode = OpFOrdGreaterThan;
    break;
  case FCmpInst::FCMP_OGE:
    opcode = OpFOrdGreaterThanEqual;
    break;
  case FCmpInst::FCMP_OLT:
    opcode = OpFOrdLessThan;
    break;
  case FCmpInst::FCMP_OLE:
    opcode = OpFOrdLessThanEqual;
    break;
  case FCmpInst::FCMP_UEQ:
    opcode = OpFUnordEqual;
    break;
  case FCmpInst::FCMP_UNE:
    opcode = OpFUnordNotEqual;
    break;
  case FCmpInst::FCMP_UGT:
    opcode = OpFUnordGreaterThan;
    break;
  case FCmpInst::FCMP_UGE:
    opcode = OpFUnordGreaterThanEqual;
    break;
  case FCmpInst::FCMP_ULT:
    opcode = OpFUnordLessThan;
    break;
  case FCmpInst::FCMP_ULE:
    opcode = OpFUnordLessThanEqual;
    break;
  case FCmpInst::FCMP_ORD:
    opcode = OpOrdered;
    break;
  case FCmpInst::FCMP_UNO:
    opcode = OpUnordered;
    break;
  case FCmpInst::FCMP_TRUE:
  case FCmpInst::FCMP_FALSE:
  default:
    opcode = OpFOrdEqual;
    break;
  }

  value_map[&inst] = builder.emit_binop(opcode, result_type, lhs, rhs);
}

void Writer::lower_cast(const CastInst &inst) {
  auto src = lower_value(inst.getOperand(0));
  auto result_type = convert_type(inst.getType());

  Op opcode;
  switch (inst.getOpcode()) {
  case Instruction::Trunc:
    opcode = OpUConvert;
    break;
  case Instruction::ZExt:
    opcode = OpUConvert;
    break;
  case Instruction::SExt:
    opcode = OpSConvert;
    break;
  case Instruction::FPToUI:
    opcode = OpConvertFToU;
    break;
  case Instruction::FPToSI:
    opcode = OpConvertFToS;
    break;
  case Instruction::UIToFP:
    opcode = OpConvertUToF;
    break;
  case Instruction::SIToFP:
    opcode = OpConvertSToF;
    break;
  case Instruction::FPTrunc:
    opcode = OpFConvert;
    break;
  case Instruction::FPExt:
    opcode = OpFConvert;
    break;
  case Instruction::BitCast:
    opcode = OpBitcast;
    break;
  case Instruction::PtrToInt:
  case Instruction::IntToPtr:
    opcode = OpBitcast;
    break;
  case Instruction::AddrSpaceCast:
    value_map[&inst] = src;
    return;
  default:
    opcode = OpBitcast;
    break;
  }

  value_map[&inst] = builder.emit_convert(opcode, result_type, src);
}

void Writer::lower_select(const SelectInst &inst) {
  auto cond = lower_value(inst.getCondition());
  auto true_val = lower_value(inst.getTrueValue());
  auto false_val = lower_value(inst.getFalseValue());
  auto result_type = convert_type(inst.getType());
  value_map[&inst] =
      builder.emit_select(result_type, cond, true_val, false_val);
}

void Writer::lower_phi(const PHINode &inst) {
  auto result_type = convert_type(inst.getType());
  std::vector<std::pair<Id, Id>> incoming;
  for (unsigned i = 0; i < inst.getNumIncomingValues(); ++i) {
    auto val_id = lower_value(inst.getIncomingValue(i));
    auto block_id = block_map[inst.getIncomingBlock(i)];
    incoming.push_back({val_id, block_id});
  }
  value_map[&inst] = builder.emit_phi(result_type, incoming);
}

void Writer::lower_extractelement(const ExtractElementInst &inst) {
  auto vec = lower_value(inst.getVectorOperand());
  auto result_type = convert_type(inst.getType());
  if (auto *ci = dyn_cast<ConstantInt>(inst.getIndexOperand())) {
    value_map[&inst] = builder.emit_composite_extract(
        result_type, vec, {uint32_t(ci->getZExtValue())});
  } else {
    auto idx = lower_value(inst.getIndexOperand());
    value_map[&inst] =
        builder.emit_binop(OpVectorExtractDynamic, result_type, vec, idx);
  }
}

void Writer::lower_insertelement(const InsertElementInst &inst) {
  auto vec = lower_value(inst.getOperand(0));
  auto elem = lower_value(inst.getOperand(1));
  auto result_type = convert_type(inst.getType());
  if (auto *ci = dyn_cast<ConstantInt>(inst.getOperand(2))) {
    value_map[&inst] = builder.emit_composite_insert(
        result_type, elem, vec, {uint32_t(ci->getZExtValue())});
  } else {
    auto idx = lower_value(inst.getOperand(2));
    value_map[&inst] =
        builder.emit_ternop(OpVectorInsertDynamic, result_type, vec, elem, idx);
  }
}

void Writer::lower_shufflevector(const ShuffleVectorInst &inst) {
  auto v1 = lower_value(inst.getOperand(0));
  auto v2 = lower_value(inst.getOperand(1));
  auto result_type = convert_type(inst.getType());

  auto *result_vt = cast<FixedVectorType>(inst.getType());
  unsigned num_elems = result_vt->getNumElements();
  std::vector<uint32_t> components;
  for (unsigned i = 0; i < num_elems; ++i) {
    int m = inst.getMaskValue(i);
    components.push_back(m < 0 ? 0xFFFFFFFF : uint32_t(m));
  }

  value_map[&inst] =
      builder.emit_vector_shuffle(result_type, v1, v2, components);
}

void Writer::lower_extractvalue(const ExtractValueInst &inst) {
  auto agg = lower_value(inst.getAggregateOperand());
  auto result_type = convert_type(inst.getType());
  std::vector<uint32_t> idxs(inst.idx_begin(), inst.idx_end());
  value_map[&inst] = builder.emit_composite_extract(result_type, agg, idxs);
}

void Writer::lower_insertvalue(const InsertValueInst &inst) {
  auto agg = lower_value(inst.getAggregateOperand());
  auto val = lower_value(inst.getInsertedValueOperand());
  auto result_type = convert_type(inst.getType());
  std::vector<uint32_t> idxs(inst.idx_begin(), inst.idx_end());
  value_map[&inst] =
      builder.emit_composite_insert(result_type, val, agg, idxs);
}

void Writer::lower_return(const ReturnInst &inst) {
  if (auto *ret_val = inst.getReturnValue()) {
    auto val_id = lower_value(ret_val);

    if (builder.shader_type == MTL2SPV_SHADER_VERTEX && output_position_var) {
      auto *ret_type = ret_val->getType();
      if (auto *st = dyn_cast<StructType>(ret_type)) {
        // First vector field is typically position
        for (unsigned i = 0; i < st->getNumElements(); ++i) {
          auto elem_id = builder.emit_composite_extract(
              convert_type(st->getElementType(i)), val_id, {i});
          if (i == 0 && st->getElementType(i)->isVectorTy()) {
            builder.emit_store(output_position_var, elem_id);
          }
          if (i < output_vars.size()) {
            builder.emit_store(output_vars[i].second, elem_id);
          }
        }
      } else if (ret_type->isVectorTy()) {
        builder.emit_store(output_position_var, val_id);
      }
    } else if (builder.shader_type == MTL2SPV_SHADER_FRAGMENT) {
      auto *ret_type = ret_val->getType();
      if (auto *st = dyn_cast<StructType>(ret_type)) {
        for (unsigned i = 0; i < st->getNumElements() && i < output_vars.size();
             ++i) {
          auto elem_id = builder.emit_composite_extract(
              convert_type(st->getElementType(i)), val_id, {i});
          builder.emit_store(output_vars[i].second, elem_id);
        }
      } else if (!output_vars.empty()) {
        builder.emit_store(output_vars[0].second, val_id);
      }
    }
  }
  builder.emit_return();
}

void Writer::lower_branch(const BranchInst &inst) {
  if (inst.isUnconditional()) {
    builder.emit_branch(block_map[inst.getSuccessor(0)]);
  } else {
    auto cond = lower_value(inst.getCondition());
    auto true_block = block_map[inst.getSuccessor(0)];
    auto false_block = block_map[inst.getSuccessor(1)];

    // Find the merge block: the immediate post-dominator of the branch.
    // For now, use a simple heuristic: if the two successors share a
    // common successor, use that as merge. Otherwise use the false block.
    Id merge_id = false_block;
    auto *true_bb = inst.getSuccessor(0);
    auto *false_bb = inst.getSuccessor(1);
    auto *true_term = true_bb->getTerminator();
    if (true_term && true_term->getNumSuccessors() == 1) {
      auto it = block_map.find(true_term->getSuccessor(0));
      if (it != block_map.end())
        merge_id = it->second;
    }

    builder.emit_selection_merge(merge_id);
    builder.emit_branch_conditional(cond, true_block, false_block);
  }
}

void Writer::lower_switch(const SwitchInst &inst) {
  auto cond = lower_value(inst.getCondition());
  auto default_label = block_map[inst.getDefaultDest()];
  std::vector<std::pair<uint32_t, Id>> cases;
  for (auto &c : inst.cases()) {
    cases.push_back(
        {uint32_t(c.getCaseValue()->getZExtValue()),
         block_map[c.getCaseSuccessor()]});
  }
  auto merge_id = builder.make_id();
  builder.emit_selection_merge(merge_id);
  builder.emit_switch(cond, default_label, cases);
}

void Writer::lower_call(const CallInst &inst) {
  auto *callee = inst.getCalledFunction();
  if (!callee)
    return;

  auto callee_name = callee->getName();

  if (callee_name.startswith("air.") || callee_name.startswith("_air.")) {
    auto name = callee_name.startswith("_air.") ? callee_name.drop_front(1)
                                                 : callee_name;
    auto result = lower_air_intrinsic(inst, name);
    if (result != 0 && !inst.getType()->isVoidTy())
      value_map[&inst] = result;
    return;
  }

  if (callee_name.startswith("llvm.")) {
    auto result = lower_llvm_intrinsic(inst, callee_name);
    if (result != 0 && !inst.getType()->isVoidTy())
      value_map[&inst] = result;
    return;
  }

  // Regular function call - should not happen in typical Metal shaders
  // since they get inlined, but handle it
  auto result_type = convert_type(inst.getType());
  std::vector<Id> args;
  for (unsigned i = 0; i < inst.arg_size(); ++i) {
    args.push_back(lower_value(inst.getArgOperand(i)));
  }

  if (!inst.getType()->isVoidTy()) {
    value_map[&inst] = builder.emit_function_call(result_type, 0, args);
  }
}

// ---- AIR intrinsic lowering ----

Id Writer::lower_air_intrinsic(const CallInst &call, StringRef name) {
  if (name.startswith("air.sample_texture") ||
      name.startswith("air.sample_depth"))
    return lower_air_sample(call, name);
  if (name.startswith("air.read_texture"))
    return lower_air_read_texture(call, name);
  if (name.startswith("air.write_texture"))
    return lower_air_write_texture(call, name);
  if (name.contains("barrier"))
    return lower_air_barrier(call, name);

  return lower_air_math(call, name);
}

Id Writer::lower_air_sample(const CallInst &call, StringRef name) {
  // air.sample_texture_2d.v4f32(texture, sampler, coord, ...)
  if (call.arg_size() < 3)
    return 0;

  auto tex_id = lower_value(call.getArgOperand(0));
  auto samp_id = lower_value(call.getArgOperand(1));
  auto coord_id = lower_value(call.getArgOperand(2));

  auto f32_ty = builder.float_type(32);
  auto v4f32 = builder.vector_type(f32_ty, 4);

  auto img_ty = v4f32;
  auto result_type = convert_type(call.getType());

  auto tex_loaded = builder.emit_load(img_ty, tex_id);
  auto samp_loaded = builder.emit_load(builder.sampler_type(), samp_id);

  // Get the image type for sampled image creation
  // We need to find the texture's image type
  Id img_type_id = 0;
  for (auto &[idx, itype] : texture_image_types) {
    if (texture_vars.count(idx) && texture_vars[idx] == tex_id) {
      img_type_id = itype;
      break;
    }
  }

  if (img_type_id == 0) {
    img_type_id = builder.image_type(f32_ty, Dim2D, 0, 0, 0, 1,
                                     ImageFormatUnknown);
  }

  tex_loaded = builder.emit_load(img_type_id, tex_id);
  auto sampled_img_type = builder.sampled_image_type(img_type_id);
  auto sampled_img =
      builder.emit_sampled_image(sampled_img_type, tex_loaded, samp_loaded);

  bool is_depth = name.contains("depth");
  if (is_depth && call.arg_size() > 3) {
    auto dref = lower_value(call.getArgOperand(3));
    return builder.emit_image_sample_dref_implicit_lod(
        result_type, sampled_img, coord_id, dref);
  }

  uint32_t image_ops = 0;
  std::vector<Id> extra_ops;

  if (name.contains("_lod") && call.arg_size() > 3) {
    image_ops |= ImageOperandsLodMask;
    extra_ops.push_back(lower_value(call.getArgOperand(3)));
    return builder.emit_image_sample_explicit_lod(result_type, sampled_img,
                                                  coord_id, image_ops,
                                                  extra_ops);
  }

  if (name.contains("_grad") && call.arg_size() > 4) {
    image_ops |= ImageOperandsGradMask;
    extra_ops.push_back(lower_value(call.getArgOperand(3)));
    extra_ops.push_back(lower_value(call.getArgOperand(4)));
    return builder.emit_image_sample_explicit_lod(result_type, sampled_img,
                                                  coord_id, image_ops,
                                                  extra_ops);
  }

  if (name.contains("_bias") && call.arg_size() > 3) {
    image_ops |= ImageOperandsBiasMask;
    extra_ops.push_back(lower_value(call.getArgOperand(3)));
  }

  return builder.emit_image_sample_implicit_lod(result_type, sampled_img,
                                                coord_id, image_ops,
                                                extra_ops);
}

Id Writer::lower_air_read_texture(const CallInst &call, StringRef name) {
  if (call.arg_size() < 2)
    return 0;

  auto tex_id = lower_value(call.getArgOperand(0));
  auto coord_id = lower_value(call.getArgOperand(1));
  auto result_type = convert_type(call.getType());

  auto f32_ty = builder.float_type(32);
  Id img_type_id = 0;
  for (auto &[idx, itype] : texture_image_types) {
    if (texture_vars.count(idx) && texture_vars[idx] == tex_id) {
      img_type_id = itype;
      break;
    }
  }
  if (img_type_id == 0)
    img_type_id = builder.image_type(f32_ty, Dim2D, 0, 0, 0, 1,
                                     ImageFormatUnknown);

  auto img = builder.emit_load(img_type_id, tex_id);

  uint32_t image_ops = 0;
  std::vector<Id> extra_ops;
  if (name.contains("_lod") && call.arg_size() > 2) {
    image_ops |= ImageOperandsLodMask;
    extra_ops.push_back(lower_value(call.getArgOperand(2)));
  }

  return builder.emit_image_fetch(result_type, img, coord_id, image_ops,
                                  extra_ops);
}

Id Writer::lower_air_write_texture(const CallInst &call, StringRef name) {
  if (call.arg_size() < 3)
    return 0;

  auto tex_id = lower_value(call.getArgOperand(0));
  auto coord_id = lower_value(call.getArgOperand(1));
  auto texel_id = lower_value(call.getArgOperand(2));

  auto f32_ty = builder.float_type(32);
  Id img_type_id = 0;
  for (auto &[idx, itype] : texture_image_types) {
    if (texture_vars.count(idx) && texture_vars[idx] == tex_id) {
      img_type_id = itype;
      break;
    }
  }
  if (img_type_id == 0)
    img_type_id = builder.image_type(f32_ty, Dim2D, 0, 0, 0, 2,
                                     ImageFormatUnknown);

  auto img = builder.emit_load(img_type_id, tex_id);
  builder.emit_image_write(img, coord_id, texel_id);
  return 0;
}

Id Writer::lower_air_math(const CallInst &call, StringRef name) {
  auto result_type = convert_type(call.getType());

  struct MathIntrinsic {
    const char *prefix;
    GLSLstd450 glsl_op;
    unsigned min_args;
  };

  static const MathIntrinsic math_ops[] = {
      {"air.sin.", GLSLstd450Sin, 1},
      {"air.cos.", GLSLstd450Cos, 1},
      {"air.tan.", GLSLstd450Tan, 1},
      {"air.asin.", GLSLstd450Asin, 1},
      {"air.acos.", GLSLstd450Acos, 1},
      {"air.atan.", GLSLstd450Atan, 1},
      {"air.atan2.", GLSLstd450Atan2, 2},
      {"air.sinh.", GLSLstd450Sinh, 1},
      {"air.cosh.", GLSLstd450Cosh, 1},
      {"air.tanh.", GLSLstd450Tanh, 1},
      {"air.exp.", GLSLstd450Exp, 1},
      {"air.exp2.", GLSLstd450Exp2, 1},
      {"air.log.", GLSLstd450Log, 1},
      {"air.log2.", GLSLstd450Log2, 1},
      {"air.sqrt.", GLSLstd450Sqrt, 1},
      {"air.rsqrt.", GLSLstd450InverseSqrt, 1},
      {"air.fabs.", GLSLstd450FAbs, 1},
      {"air.abs.", GLSLstd450SAbs, 1},
      {"air.sign.", GLSLstd450FSign, 1},
      {"air.floor.", GLSLstd450Floor, 1},
      {"air.ceil.", GLSLstd450Ceil, 1},
      {"air.trunc.", GLSLstd450Trunc, 1},
      {"air.round.", GLSLstd450Round, 1},
      {"air.rint.", GLSLstd450RoundEven, 1},
      {"air.fract.", GLSLstd450Fract, 1},
      {"air.fmin.", GLSLstd450FMin, 2},
      {"air.fmax.", GLSLstd450FMax, 2},
      {"air.min.", GLSLstd450SMin, 2},
      {"air.max.", GLSLstd450SMax, 2},
      {"air.umin.", GLSLstd450UMin, 2},
      {"air.umax.", GLSLstd450UMax, 2},
      {"air.clamp.", GLSLstd450FClamp, 3},
      {"air.fclamp.", GLSLstd450FClamp, 3},
      {"air.saturate.", GLSLstd450FClamp, 1},
      {"air.mix.", GLSLstd450FMix, 3},
      {"air.step.", GLSLstd450Step, 2},
      {"air.smoothstep.", GLSLstd450SmoothStep, 3},
      {"air.fma.", GLSLstd450Fma, 3},
      {"air.pow.", GLSLstd450Pow, 2},
      {"air.length.", GLSLstd450Length, 1},
      {"air.distance.", GLSLstd450Distance, 2},
      {"air.normalize.", GLSLstd450Normalize, 1},
      {"air.cross.", GLSLstd450Cross, 2},
      {"air.dot.", GLSLstd450Bad, 2},
      {"air.reflect.", GLSLstd450Reflect, 2},
      {"air.refract.", GLSLstd450Refract, 3},
      {"air.face_forward.", GLSLstd450FaceForward, 3},
  };

  for (const auto &op : math_ops) {
    if (!name.startswith(op.prefix))
      continue;

    if (call.arg_size() < op.min_args)
      return 0;

    if (strcmp(op.prefix, "air.dot.") == 0) {
      auto a = lower_value(call.getArgOperand(0));
      auto b = lower_value(call.getArgOperand(1));
      return builder.emit_binop(OpDot, result_type, a, b);
    }

    if (strcmp(op.prefix, "air.saturate.") == 0) {
      auto val = lower_value(call.getArgOperand(0));
      auto f32_ty = builder.float_type(32);
      auto zero = builder.float_constant(f32_ty, 0.0f);
      auto one = builder.float_constant(f32_ty, 1.0f);
      return builder.emit_ext_inst(result_type, glsl_ext_inst,
                                   GLSLstd450FClamp, {val, zero, one});
    }

    std::vector<Id> operands;
    for (unsigned i = 0; i < call.arg_size() && i < op.min_args; ++i)
      operands.push_back(lower_value(call.getArgOperand(i)));

    return builder.emit_ext_inst(result_type, glsl_ext_inst, op.glsl_op,
                                 operands);
  }

  // Derivative operations
  if (name.startswith("air.dpdx.") || name.startswith("air.dfDx.")) {
    return builder.emit_unop(OpDPdx, result_type,
                             lower_value(call.getArgOperand(0)));
  }
  if (name.startswith("air.dpdy.") || name.startswith("air.dfDy.")) {
    return builder.emit_unop(OpDPdy, result_type,
                             lower_value(call.getArgOperand(0)));
  }
  if (name.startswith("air.fwidth.")) {
    return builder.emit_unop(OpFwidth, result_type,
                             lower_value(call.getArgOperand(0)));
  }

  // Discard
  if (name.startswith("air.discard_fragment")) {
    builder.emit_kill();
    return 0;
  }

  // Fallback: if we can't handle it, return the first argument's value
  if (call.arg_size() > 0 && !call.getType()->isVoidTy())
    return lower_value(call.getArgOperand(0));

  return 0;
}

Id Writer::lower_air_barrier(const CallInst &call, StringRef name) {
  auto u32_ty = builder.int_type(32, false);
  auto scope_workgroup = builder.uint_constant(u32_ty, ScopeWorkgroup);
  auto scope_device = builder.uint_constant(u32_ty, ScopeDevice);
  auto semantics = builder.uint_constant(
      u32_ty, MemorySemanticsWorkgroupMemoryMask |
                  MemorySemanticsAcquireReleaseMask);

  if (name.contains("threadgroup") || name.contains("wg")) {
    builder.emit_control_barrier(scope_workgroup, scope_workgroup, semantics);
  } else {
    builder.emit_control_barrier(scope_workgroup, scope_device, semantics);
  }
  return 0;
}

Id Writer::lower_llvm_intrinsic(const CallInst &call, StringRef name) {
  auto result_type = convert_type(call.getType());

  if (name.startswith("llvm.fabs.")) {
    return builder.emit_ext_inst(result_type, glsl_ext_inst, GLSLstd450FAbs,
                                 {lower_value(call.getArgOperand(0))});
  }
  if (name.startswith("llvm.sqrt.")) {
    return builder.emit_ext_inst(result_type, glsl_ext_inst, GLSLstd450Sqrt,
                                 {lower_value(call.getArgOperand(0))});
  }
  if (name.startswith("llvm.floor.")) {
    return builder.emit_ext_inst(result_type, glsl_ext_inst, GLSLstd450Floor,
                                 {lower_value(call.getArgOperand(0))});
  }
  if (name.startswith("llvm.ceil.")) {
    return builder.emit_ext_inst(result_type, glsl_ext_inst, GLSLstd450Ceil,
                                 {lower_value(call.getArgOperand(0))});
  }
  if (name.startswith("llvm.trunc.")) {
    return builder.emit_ext_inst(result_type, glsl_ext_inst, GLSLstd450Trunc,
                                 {lower_value(call.getArgOperand(0))});
  }
  if (name.startswith("llvm.sin.")) {
    return builder.emit_ext_inst(result_type, glsl_ext_inst, GLSLstd450Sin,
                                 {lower_value(call.getArgOperand(0))});
  }
  if (name.startswith("llvm.cos.")) {
    return builder.emit_ext_inst(result_type, glsl_ext_inst, GLSLstd450Cos,
                                 {lower_value(call.getArgOperand(0))});
  }
  if (name.startswith("llvm.pow.")) {
    return builder.emit_ext_inst(
        result_type, glsl_ext_inst, GLSLstd450Pow,
        {lower_value(call.getArgOperand(0)),
         lower_value(call.getArgOperand(1))});
  }
  if (name.startswith("llvm.exp.")) {
    return builder.emit_ext_inst(result_type, glsl_ext_inst, GLSLstd450Exp,
                                 {lower_value(call.getArgOperand(0))});
  }
  if (name.startswith("llvm.exp2.")) {
    return builder.emit_ext_inst(result_type, glsl_ext_inst, GLSLstd450Exp2,
                                 {lower_value(call.getArgOperand(0))});
  }
  if (name.startswith("llvm.log.")) {
    return builder.emit_ext_inst(result_type, glsl_ext_inst, GLSLstd450Log,
                                 {lower_value(call.getArgOperand(0))});
  }
  if (name.startswith("llvm.log2.")) {
    return builder.emit_ext_inst(result_type, glsl_ext_inst, GLSLstd450Log2,
                                 {lower_value(call.getArgOperand(0))});
  }
  if (name.startswith("llvm.fma.")) {
    return builder.emit_ext_inst(
        result_type, glsl_ext_inst, GLSLstd450Fma,
        {lower_value(call.getArgOperand(0)),
         lower_value(call.getArgOperand(1)),
         lower_value(call.getArgOperand(2))});
  }
  if (name.startswith("llvm.minnum.")) {
    return builder.emit_ext_inst(
        result_type, glsl_ext_inst, GLSLstd450FMin,
        {lower_value(call.getArgOperand(0)),
         lower_value(call.getArgOperand(1))});
  }
  if (name.startswith("llvm.maxnum.")) {
    return builder.emit_ext_inst(
        result_type, glsl_ext_inst, GLSLstd450FMax,
        {lower_value(call.getArgOperand(0)),
         lower_value(call.getArgOperand(1))});
  }

  // Ignore other llvm intrinsics (lifetime, dbg, etc.)
  if (call.arg_size() > 0 && !call.getType()->isVoidTy())
    return lower_value(call.getArgOperand(0));
  return 0;
}

// ---- Function body lowering ----

void Writer::lower_function_body(const Function &func) {
  // Pre-assign SPIR-V label IDs for all basic blocks.
  for (auto &bb : func) {
    block_map[&bb] = builder.make_id();
  }

  bool is_entry = true;
  for (auto &bb : func) {
    builder.set_block(block_map[&bb]);

    // Emit PHI nodes first (SPIR-V requires OpPhi immediately after OpLabel).
    for (auto &inst : bb) {
      if (!isa<PHINode>(inst))
        break;
      lower_phi(cast<PHINode>(inst));
    }

    // Emit alloca instructions at the start of the entry block as function
    // variables, since SPIR-V requires OpVariable at the block start.
    if (is_entry) {
      for (auto &inst : bb) {
        if (isa<AllocaInst>(inst))
          lower_alloca(cast<AllocaInst>(inst));
      }
      is_entry = false;
    }

    // Emit remaining instructions.
    for (auto &inst : bb) {
      if (builder.has_error())
        return;
      if (isa<PHINode>(inst) || (isa<AllocaInst>(inst) && &bb == &func.getEntryBlock()))
        continue;
      lower_instruction(inst);
    }
  }
}

// ---- Main generation ----

llvm::Error Writer::generate() {
  const auto *entry = get_entry_function();
  if (!entry) {
    return make_error<StringError>("entry function not found",
                                   inconvertibleErrorCode());
  }
  if (entry->isDeclaration()) {
    return make_error<StringError>("entry function is declaration",
                                   inconvertibleErrorCode());
  }

  const auto execution_model = get_execution_model(builder.shader_type);
  if (execution_model == ExecutionModelMax) {
    return make_error<StringError>("unsupported shader type",
                                   inconvertibleErrorCode());
  }

  // Capabilities
  builder.add_capability(CapabilityShader);
  if (builder.shader_type == MTL2SPV_SHADER_FRAGMENT) {
    // may need InputAttachment for subpass inputs
  }
  if (builder.shader_type == MTL2SPV_SHADER_COMPUTE) {
    // may need SubgroupBallotKHR etc.
  }

  builder.set_memory_model(AddressingModelLogical, MemoryModelGLSL450);

  glsl_ext_inst = builder.import_ext_inst_set("GLSL.std.450");

  // Create interface variables from AIR metadata
  process_entry_arguments();
  create_output_variables();

  // Create wrapper function (void -> void entry point for SPIR-V)
  auto void_type_id = builder.void_type();
  auto fn_type_id = builder.function_type(void_type_id, {});
  entry_function_id = builder.begin_function(
      void_type_id, FunctionControlMaskNone, fn_type_id);

  builder.add_entry_point(execution_model, entry_function_id,
                          builder.entry_name, interface_ids);
  builder.add_name(entry_function_id, builder.entry_name);

  // Execution modes
  if (builder.shader_type == MTL2SPV_SHADER_FRAGMENT) {
    builder.add_execution_mode(entry_function_id, ExecutionModeOriginUpperLeft);
    if (has_entry_option("early_fragment_tests")) {
      builder.add_execution_mode(entry_function_id,
                                 ExecutionModeEarlyFragmentTests);
    }
    bool writes_depth = false;
    for (auto &[idx, var] : output_vars) {
      // Check if any output is depth
    }
    if (writes_depth || has_entry_option("depth_any")) {
      builder.add_execution_mode(entry_function_id,
                                 ExecutionModeDepthReplacing);
    }
  } else if (builder.shader_type == MTL2SPV_SHADER_COMPUTE) {
    const auto local_size = get_local_size();
    builder.add_execution_mode(entry_function_id, ExecutionModeLocalSize,
                               {local_size[0], local_size[1], local_size[2]});
  }

  // Lower the entry function body (handles block labels internally)
  lower_function_body(*entry);

  builder.end_function();
  builder.finalize();

  if (builder.has_error()) {
    return make_error<StringError>(builder.get_error(),
                                   inconvertibleErrorCode());
  }

  return Error::success();
}

} // namespace spv

// ---- Integration with metallib ----

namespace metal {
using namespace spv;

void Library::Function::sync_api() {
  api.name = entry_name;
  api.type = shader_type;
  api.module = module.get();
  api.buffer = buffer.get();
}

void Library::Function::parse_arguments() {
  args.clear();

  llvm::NamedMDNode *fn_list = nullptr;
  switch (shader_type) {
  case MTL2SPV_SHADER_VERTEX:
    fn_list = module->getNamedMetadata("air.vertex");
    break;
  case MTL2SPV_SHADER_FRAGMENT:
    fn_list = module->getNamedMetadata("air.fragment");
    break;
  case MTL2SPV_SHADER_COMPUTE:
    fn_list = module->getNamedMetadata("air.kernel");
    break;
  default:
    break;
  }

  if (!fn_list || fn_list->getNumOperands() == 0)
    return;

  auto *root = fn_list->getOperand(0);
  if (!root || root->getNumOperands() < 3)
    return;

  auto *arg_list = dyn_cast_or_null<llvm::MDNode>(root->getOperand(2));
  if (!arg_list)
    return;

  auto parse_access = [](llvm::MDNode *node, unsigned index) {
    if (index >= node->getNumOperands())
      return MTL2SPV_ARGUMENT_ACCESS_READ_ONLY;
    const auto *access_md =
        llvm::dyn_cast_or_null<llvm::MDString>(node->getOperand(index));
    if (!access_md)
      return MTL2SPV_ARGUMENT_ACCESS_READ_ONLY;
    const auto value = access_md->getString().str();
    if (value == "air.write")
      return MTL2SPV_ARGUMENT_ACCESS_WRITE_ONLY;
    if (value == "air.read_write")
      return MTL2SPV_ARGUMENT_ACCESS_READ_WRITE;
    return MTL2SPV_ARGUMENT_ACCESS_READ_ONLY;
  };

  auto parse_texture_type = [](const std::string &type_name) {
    if (type_name.find("texture1d_array") != std::string::npos)
      return MTL2SPV_TEXTURE_TYPE_1D_ARRAY;
    if (type_name.find("texture1d") != std::string::npos)
      return MTL2SPV_TEXTURE_TYPE_1D;
    if (type_name.find("texture2d_ms_array") != std::string::npos)
      return MTL2SPV_TEXTURE_TYPE_2D_MULTISAMPLE_ARRAY;
    if (type_name.find("texture2d_ms") != std::string::npos)
      return MTL2SPV_TEXTURE_TYPE_2D_MULTISAMPLE;
    if (type_name.find("texture2d_array") != std::string::npos)
      return MTL2SPV_TEXTURE_TYPE_2D_ARRAY;
    if (type_name.find("texturecube_array") != std::string::npos)
      return MTL2SPV_TEXTURE_TYPE_CUBE_ARRAY;
    if (type_name.find("texturecube") != std::string::npos)
      return MTL2SPV_TEXTURE_TYPE_CUBE;
    if (type_name.find("texture3d") != std::string::npos)
      return MTL2SPV_TEXTURE_TYPE_3D;
    if (type_name.find("texture_buffer") != std::string::npos)
      return MTL2SPV_TEXTURE_TYPE_TEXTURE_BUFFER;
    return MTL2SPV_TEXTURE_TYPE_2D;
  };

  for (auto &arg_parent : arg_list->operands()) {
    auto *node = dyn_cast_or_null<llvm::MDNode>(arg_parent);
    if (!node || node->getNumOperands() < 2)
      continue;

    mtl2spv_argument arg{};
    arg.access = MTL2SPV_ARGUMENT_ACCESS_READ_ONLY;

    const auto *storage_md =
        dyn_cast_or_null<llvm::MDString>(node->getOperand(1));
    if (!storage_md)
      continue;
    const auto storage_type = storage_md->getString().str();

    if (storage_type == "air.texture" && node->getNumOperands() >= 10) {
      arg.type = MTL2SPV_ARGUMENT_TYPE_TEXTURE;
      arg.index =
          int(mdconst::dyn_extract<llvm::ConstantInt>(node->getOperand(3))
                  ->getZExtValue());
      arg.array_len =
          int(mdconst::dyn_extract<llvm::ConstantInt>(node->getOperand(4))
                  ->getZExtValue());
      arg.access = parse_access(node, 5);
      const auto *type_md =
          dyn_cast_or_null<llvm::MDString>(node->getOperand(7));
      if (!type_md)
        continue;
      const auto arg_type = type_md->getString().str();
      arg.texture.texture_type = parse_texture_type(arg_type);
      arg.texture.is_depth = arg_type.find("depth") != std::string::npos;

      const auto open = arg_type.find('<');
      if (open != std::string::npos) {
        const auto end = arg_type.find_first_of(",>", open + 1);
        if (end != std::string::npos && end > open + 1) {
          arg.texture.data_type =
              convert_type(arg_type.substr(open + 1, end - open - 1));
        }
      }
      args.push_back(arg);
      continue;
    }

    if ((storage_type == "air.buffer" ||
         storage_type == "air.indirect_buffer") &&
        node->getNumOperands() >= 16) {
      arg.type = MTL2SPV_ARGUMENT_TYPE_BUFFER;
      arg.buffer.data_size =
          int(mdconst::dyn_extract<llvm::ConstantInt>(node->getOperand(9))
                  ->getSExtValue());
      arg.buffer.alignment =
          int(mdconst::dyn_extract<llvm::ConstantInt>(node->getOperand(11))
                  ->getSExtValue());
      const auto *type_md =
          dyn_cast_or_null<llvm::MDString>(node->getOperand(13));
      if (!type_md)
        continue;
      arg.buffer.data_type = convert_type(type_md->getString().str());
      arg.index =
          int(mdconst::dyn_extract<llvm::ConstantInt>(node->getOperand(5))
                  ->getZExtValue());
      arg.array_len =
          int(mdconst::dyn_extract<llvm::ConstantInt>(node->getOperand(6))
                  ->getZExtValue());
      arg.access = parse_access(node, 7);
      args.push_back(arg);
      continue;
    }

    if (storage_type == "air.sampler" && node->getNumOperands() >= 5) {
      arg.type = MTL2SPV_ARGUMENT_TYPE_SAMPLER;
      arg.index =
          int(mdconst::dyn_extract<llvm::ConstantInt>(node->getOperand(3))
                  ->getZExtValue());
      arg.array_len =
          int(mdconst::dyn_extract<llvm::ConstantInt>(node->getOperand(4))
                  ->getZExtValue());
      args.push_back(arg);
      continue;
    }
  }
}

llvm::Error Library::Function::write_spirv(const mtl2spv_options &options,
                                           spv::Blob &out_spirv) const {
  Builder builder(options, entry_name, shader_type, out_spirv);
  Writer writer(*module, builder);
  return writer.generate();
}

const uint32_t *Library::get_spirv(int index, size_t *word_count) const {
  if (word_count)
    *word_count = 0;
  if (index < 0 || index >= (int)functions.size())
    return nullptr;
  if (word_count)
    *word_count = functions[index].spirv.size();
  return functions[index].spirv.empty() ? nullptr
                                        : functions[index].spirv.data();
}

size_t Library::get_spirv_word_count(int index) const {
  if (index < 0 || index >= (int)functions.size())
    return 0;
  return functions[index].spirv.size();
}

const char *Library::get_last_error() const { return last_error.c_str(); }

mtl2spv_error Library::write_spv(const mtl2spv_options &options, int index) {
  last_error.clear();
  if (index < 0 || index >= (int)functions.size()) {
    last_error = "invalid function index";
    return MTL2SPV_SPV_WRITE_FAILED;
  }

  auto &function = functions[index];
  function.spirv.clear();
  if (auto err = function.write_spirv(options, function.spirv)) {
    last_error = llvm::toString(std::move(err));
    function.spirv.clear();
    return MTL2SPV_SPV_WRITE_FAILED;
  }

  return MTL2SPV_SUCCESS;
}

mtl2spv_error Library::write_spv(const mtl2spv_options &options,
                                 const String &name) {
  auto iter = map.find(name);
  if (iter == map.end()) {
    last_error = "entry function not found";
    return MTL2SPV_SPV_WRITE_FAILED;
  }
  return write_spv(options, int(iter->second));
}
} // namespace metal
