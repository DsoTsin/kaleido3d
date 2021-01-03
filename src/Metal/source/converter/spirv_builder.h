#pragma once

#include "spirv_util.h"
#include <unordered_map>

namespace spv {

class Builder {
public:
  Builder(const mtl2spv_options &options, const metal::String &entry_name,
          const mtl2spv_shader_type &shader_type, Blob &blob);
  ~Builder() = default;

  Id make_id();

  // ---- Type creation ----

  Id void_type();
  Id bool_type();
  Id int_type(unsigned width, bool is_signed);
  Id float_type(unsigned width);
  Id vector_type(Id component_type, unsigned count);
  Id matrix_type(Id column_type, unsigned column_count);
  Id array_type(Id element_type, Id length_id);
  Id runtime_array_type(Id element_type);
  Id struct_type(const std::vector<Id> &member_types);
  Id pointer_type(Id pointee_type, StorageClass storage);
  Id function_type(Id return_type, const std::vector<Id> &param_types);
  Id image_type(Id sampled_type, Dim dim, uint32_t depth, uint32_t arrayed,
                uint32_t ms, uint32_t sampled, ImageFormat format);
  Id sampled_image_type(Id image_type_id);
  Id sampler_type();

  // ---- Constants ----

  Id bool_constant(bool value);
  Id int_constant(Id type, int32_t value);
  Id uint_constant(Id type, uint32_t value);
  Id int64_constant(Id type, int64_t value);
  Id uint64_constant(Id type, uint64_t value);
  Id float_constant(Id type, float value);
  Id double_constant(Id type, double value);
  Id null_constant(Id type);
  Id composite_constant(Id type, const std::vector<Id> &constituents);

  // ---- Variables ----

  Id global_variable(Id ptr_type, StorageClass storage, Id initializer = 0);
  Id function_variable(Id ptr_type);

  // ---- Extended instruction sets ----

  Id import_ext_inst_set(const char *name);

  // ---- Decorations ----

  void decorate(Id target, Decoration decoration,
                const std::vector<uint32_t> &literals = {});
  void member_decorate(Id struct_type, uint32_t member,
                       Decoration decoration,
                       const std::vector<uint32_t> &literals = {});

  // ---- Module-level declarations ----

  void add_capability(Capability capability);
  void set_memory_model(AddressingModel addressing_model,
                        MemoryModel memory_model);
  void add_entry_point(ExecutionModel model, Id function_id,
                       const metal::String &name,
                       const std::vector<Id> &interface_ids = {});
  void add_execution_mode(Id entry_point, ExecutionMode mode,
                          std::initializer_list<uint32_t> literals = {});
  void add_name(Id target_id, const metal::String &name);
  void add_name(Id target_id, const char *name);
  void add_member_name(Id struct_type_id, uint32_t member, const char *name);

  // ---- Function structure ----

  Id begin_function(Id return_type, FunctionControlMask control, Id fn_type);
  Id begin_block();
  void set_block(Id label_id);
  void end_function();
  Id begin_function_param(Id param_type);

  // ---- Instructions ----

  Id emit_load(Id result_type, Id pointer, uint32_t memory_access = 0);
  void emit_store(Id pointer, Id value, uint32_t memory_access = 0);
  Id emit_access_chain(Id result_type, Id base, const std::vector<Id> &indexes);
  Id emit_in_bounds_access_chain(Id result_type, Id base,
                                  const std::vector<Id> &indexes);

  Id emit_binop(Op opcode, Id result_type, Id operand1, Id operand2);
  Id emit_unop(Op opcode, Id result_type, Id operand);
  Id emit_ternop(Op opcode, Id result_type, Id a, Id b, Id c);

  Id emit_composite_extract(Id result_type, Id composite,
                            const std::vector<uint32_t> &indexes);
  Id emit_composite_construct(Id result_type,
                              const std::vector<Id> &constituents);
  Id emit_composite_insert(Id result_type, Id object, Id composite,
                           const std::vector<uint32_t> &indexes);
  Id emit_vector_shuffle(Id result_type, Id vec1, Id vec2,
                         const std::vector<uint32_t> &components);
  Id emit_select(Id result_type, Id condition, Id true_val, Id false_val);
  Id emit_phi(Id result_type,
              const std::vector<std::pair<Id, Id>> &incoming);
  Id emit_ext_inst(Id result_type, Id set, uint32_t instruction,
                   const std::vector<Id> &operands);
  Id emit_convert(Op opcode, Id result_type, Id value);
  Id emit_bitcast(Id result_type, Id value);

  // ---- Image instructions ----

  Id emit_sampled_image(Id result_type, Id image, Id sampler);
  Id emit_image_sample_implicit_lod(
      Id result_type, Id sampled_image, Id coordinate,
      uint32_t image_operands = 0,
      const std::vector<Id> &operand_ids = {});
  Id emit_image_sample_explicit_lod(
      Id result_type, Id sampled_image, Id coordinate,
      uint32_t image_operands, const std::vector<Id> &operand_ids);
  Id emit_image_sample_dref_implicit_lod(
      Id result_type, Id sampled_image, Id coordinate, Id dref,
      uint32_t image_operands = 0,
      const std::vector<Id> &operand_ids = {});
  Id emit_image_fetch(Id result_type, Id image, Id coordinate,
                      uint32_t image_operands = 0,
                      const std::vector<Id> &operand_ids = {});
  Id emit_image_read(Id result_type, Id image, Id coordinate);
  void emit_image_write(Id image, Id coordinate, Id texel);
  Id emit_image(Id result_type, Id sampled_image);
  Id emit_image_query_size_lod(Id result_type, Id image, Id lod);
  Id emit_image_query_size(Id result_type, Id image);
  Id emit_image_query_levels(Id result_type, Id image);
  Id emit_image_query_samples(Id result_type, Id image);

  // ---- Control flow ----

  void emit_branch(Id target_label);
  void emit_branch_conditional(Id condition, Id true_label, Id false_label,
                               const std::vector<uint32_t> &weights = {});
  void emit_selection_merge(Id merge_block,
                            SelectionControlMask control = SelectionControlMaskNone);
  void emit_loop_merge(Id merge_block, Id continue_block,
                       LoopControlMask control = LoopControlMaskNone);
  void emit_switch(Id selector, Id default_label,
                   const std::vector<std::pair<uint32_t, Id>> &cases);
  void emit_return();
  void emit_return_value(Id value);
  void emit_kill();
  void emit_unreachable();

  // ---- Function calls ----

  Id emit_function_call(Id result_type, Id function,
                        const std::vector<Id> &args);

  // ---- Atomic / barrier ----

  void emit_control_barrier(Id execution, Id memory, Id semantics);
  void emit_memory_barrier(Id memory, Id semantics);

  // ---- Finalization ----

  void finalize();
  void fail(const std::string &message);
  bool has_error() const { return !error.empty(); }
  const std::string &get_error() const { return error; }

  friend class Writer;

private:
  void append_inst(std::vector<uint32_t> &section, Op op,
                   std::initializer_list<uint32_t> operands);
  void append_inst(std::vector<uint32_t> &section, Op op,
                   const std::vector<uint32_t> &operands);
  static void append_string(std::vector<uint32_t> &words,
                            const metal::String &value);
  static void append_string(std::vector<uint32_t> &words, const char *value);

  Blob &blob;
  const mtl2spv_options &options;
  const metal::String &entry_name;
  const mtl2spv_shader_type &shader_type;
  Id next_id;
  bool module_finalized;
  std::string error;

  // SPIR-V module sections (emitted in order)
  std::vector<uint32_t> capabilities;
  std::vector<uint32_t> extensions;
  std::vector<uint32_t> ext_imports;
  std::vector<uint32_t> memory_model_section;
  std::vector<uint32_t> entry_points;
  std::vector<uint32_t> execution_modes;
  std::vector<uint32_t> debug;
  std::vector<uint32_t> annotations;
  std::vector<uint32_t> types_globals;
  std::vector<uint32_t> functions;

  // Type caches
  Id void_type_id;
  Id bool_type_id;
  Id sampler_type_id;

  struct TypeKey {
    uint64_t a, b;
    bool operator==(const TypeKey &o) const { return a == o.a && b == o.b; }
  };
  struct TypeKeyHash {
    size_t operator()(const TypeKey &k) const {
      return std::hash<uint64_t>()(k.a) ^ (std::hash<uint64_t>()(k.b) << 1);
    }
  };

  std::unordered_map<TypeKey, Id, TypeKeyHash> type_cache;
  std::unordered_map<uint64_t, Id> const_cache;
  std::unordered_map<Id, bool> int_signedness;
};

} // namespace spv
