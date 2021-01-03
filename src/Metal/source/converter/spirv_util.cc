#include "spirv_util.h"

namespace spv {
using namespace llvm;

static std::unordered_map<std::string, mtl2spv_data_type> type_map = {
    {"char", MTL2SPV_DATA_TYPE_CHAR},
    {"int8_t", MTL2SPV_DATA_TYPE_CHAR},
    {"unsigned char", MTL2SPV_DATA_TYPE_UCHAR},
    {"uchar", MTL2SPV_DATA_TYPE_UCHAR},
    {"uint8_t", MTL2SPV_DATA_TYPE_UCHAR},
    {"short", MTL2SPV_DATA_TYPE_SHORT},
    {"int16_t", MTL2SPV_DATA_TYPE_SHORT},
    {"unsigned short", MTL2SPV_DATA_TYPE_USHORT},
    {"ushort", MTL2SPV_DATA_TYPE_USHORT},
    {"uint16_t", MTL2SPV_DATA_TYPE_USHORT},
    {"half", MTL2SPV_DATA_TYPE_HALF},
    {"half2", MTL2SPV_DATA_TYPE_HALF2},
    {"half3", MTL2SPV_DATA_TYPE_HALF3},
    {"half4", MTL2SPV_DATA_TYPE_HALF4},
    {"float", MTL2SPV_DATA_TYPE_FLOAT},
    {"float2", MTL2SPV_DATA_TYPE_FLOAT2},
    {"float3", MTL2SPV_DATA_TYPE_FLOAT3},
    {"float4", MTL2SPV_DATA_TYPE_FLOAT4},
    {"int", MTL2SPV_DATA_TYPE_INT},
    {"int32_t", MTL2SPV_DATA_TYPE_INT},
    {"int2", MTL2SPV_DATA_TYPE_INT2},
    {"int3", MTL2SPV_DATA_TYPE_INT3},
    {"int4", MTL2SPV_DATA_TYPE_INT4},
    {"uint", MTL2SPV_DATA_TYPE_UINT},
    {"uint2", MTL2SPV_DATA_TYPE_UINT2},
    {"uint3", MTL2SPV_DATA_TYPE_UINT3},
    {"uint4", MTL2SPV_DATA_TYPE_UINT4},
    {"bool", MTL2SPV_DATA_TYPE_BOOL},
    {"bool2", MTL2SPV_DATA_TYPE_BOOL2},
    {"bool3", MTL2SPV_DATA_TYPE_BOOL3},
    {"bool4", MTL2SPV_DATA_TYPE_BOOL4},
};

mtl2spv_data_type convert_type(std::string const &type_name) {
  auto iter = type_map.find(type_name);
  if (iter == type_map.end())
    return MTL2SPV_DATA_TYPE_NONE;
  return iter->second;
}

int64_t cast_operand_to_int64(MDNode *N, unsigned I) {
  return mdconst::dyn_extract<ConstantInt>(N->getOperand(I))->getZExtValue();
}

llvm::Metadata *cast_operand_or_null(llvm::MDNode *N, unsigned I) {
  if (!N)
    return nullptr;
  return N->getOperand(I);
}

std::string cast_operand_to_string(MDNode *N, unsigned I) {
  if (auto *Str = dyn_cast_or_null<MDString>(cast_operand_or_null(N, I)))
    return Str->getString().str();
  return "";
}

MDNode *cast_operand_to_mdnode(MDNode *N, unsigned I) {
  return dyn_cast_or_null<MDNode>(cast_operand_or_null(N, I));
}

Type *cast_operand_to_type(MDNode *N, unsigned I) {
  return cast<ValueAsMetadata>(N->getOperand(I))->getType();
}

std::set<std::string> getNamedMDAsStringSet(Module *M,
                                            const std::string &MDName) {
  NamedMDNode *NamedMD = M->getNamedMetadata(MDName);
  std::set<std::string> StrSet;
  if (!NamedMD)
    return StrSet;

  for (unsigned I = 0, E = NamedMD->getNumOperands(); I != E; ++I) {
    MDNode *MD = NamedMD->getOperand(I);
    if (!MD || MD->getNumOperands() == 0)
      continue;
    for (unsigned J = 0, N = MD->getNumOperands(); J != N; ++J)
      StrSet.insert(cast_operand_to_string(MD, J));
  }

  return StrSet;
}

// ---- AIR ↔ SPIR-V mapping helpers ----

StorageClass air_address_space_to_spirv(unsigned addr_space) {
  switch (addr_space) {
  case 0:
    return StorageClassFunction;
  case 1:
    return StorageClassStorageBuffer;
  case 2:
    return StorageClassUniform;
  case 3:
    return StorageClassWorkgroup;
  default:
    return StorageClassFunction;
  }
}

BuiltIn air_builtin_to_spirv(llvm::StringRef name,
                             mtl2spv_shader_type shader_type) {
  // Vertex builtins
  if (name == "air.vertex_id")
    return BuiltInVertexIndex;
  if (name == "air.instance_id")
    return BuiltInInstanceIndex;
  if (name == "air.base_vertex")
    return BuiltInBaseVertex;
  if (name == "air.base_instance")
    return BuiltInBaseInstance;
  if (name == "air.position") {
    if (shader_type == MTL2SPV_SHADER_VERTEX)
      return BuiltInPosition;
    return BuiltInFragCoord;
  }
  if (name == "air.point_size")
    return BuiltInPointSize;
  if (name == "air.clip_distance")
    return BuiltInClipDistance;
  if (name == "air.render_target_array_index")
    return BuiltInLayer;
  if (name == "air.viewport_array_index")
    return BuiltInViewportIndex;

  // Fragment builtins
  if (name == "air.front_facing")
    return BuiltInFrontFacing;
  if (name == "air.point_coord")
    return BuiltInPointCoord;
  if (name == "air.sample_id")
    return BuiltInSampleId;
  if (name == "air.sample_mask_in" || name == "air.sample_mask")
    return BuiltInSampleMask;
  if (name == "air.depth")
    return BuiltInFragDepth;

  // Compute builtins
  if (name == "air.thread_position_in_grid")
    return BuiltInGlobalInvocationId;
  if (name == "air.thread_position_in_threadgroup")
    return BuiltInLocalInvocationId;
  if (name == "air.threadgroup_position_in_grid")
    return BuiltInWorkgroupId;
  if (name == "air.threads_per_threadgroup")
    return BuiltInWorkgroupSize;
  if (name == "air.threadgroups_per_grid")
    return BuiltInNumWorkgroups;
  if (name == "air.thread_index_in_threadgroup")
    return BuiltInLocalInvocationIndex;
  if (name == "air.threads_per_simdgroup" ||
      name == "air.thread_execution_width")
    return BuiltInSubgroupSize;
  if (name == "air.thread_index_in_simdgroup")
    return BuiltInSubgroupLocalInvocationId;
  if (name == "air.simdgroup_index_in_threadgroup")
    return BuiltInSubgroupId;
  if (name == "air.simdgroups_per_threadgroup")
    return BuiltInNumSubgroups;
  if (name == "air.thread_index_in_quadgroup")
    return BuiltInSubgroupLocalInvocationId;

  // Tessellation
  if (name == "air.patch_id")
    return BuiltInPatchVertices;

  return BuiltInMax;
}

Dim air_texture_dim(llvm::StringRef type_name) {
  if (type_name.contains("texture3d"))
    return Dim3D;
  if (type_name.contains("texturecube"))
    return DimCube;
  if (type_name.contains("texture_buffer"))
    return DimBuffer;
  if (type_name.contains("texture1d"))
    return Dim1D;
  return Dim2D;
}

bool air_texture_arrayed(llvm::StringRef type_name) {
  return type_name.contains("_array");
}

bool air_texture_ms(llvm::StringRef type_name) {
  return type_name.contains("_ms");
}

bool air_is_depth_texture(llvm::StringRef type_name) {
  return type_name.contains("depth");
}

} // namespace spv
