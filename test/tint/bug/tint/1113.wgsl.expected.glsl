//
// main_count
//
#version 310 es


struct Dbg {
  uint offsetCounter;
  uint pad0;
  uint pad1;
  uint pad2;
  uint value0;
  uint value1;
  uint value2;
  uint value3;
  float value_f32_0;
  float value_f32_1;
  float value_f32_2;
  float value_f32_3;
};

layout(binding = 0, std140)
uniform uniforms_block_1_ubo {
  uvec4 inner[3];
} v;
layout(binding = 1, std430)
buffer U32s_1_ssbo {
  uint values[];
} indices;
layout(binding = 2, std430)
buffer F32s_1_ssbo {
  float values[];
} positions;
layout(binding = 3, std430)
buffer AU32s_1_ssbo {
  uint values[];
} counters;
layout(binding = 4, std430)
buffer AI32s_1_ssbo {
  int values[];
} LUT;
layout(binding = 5, std430)
buffer dbg_block_1_ssbo {
  Dbg inner;
} v_1;
vec3 toVoxelPos(vec3 position) {
  uvec4 v_2 = v.inner[1u];
  uvec4 v_3 = v.inner[1u];
  uvec4 v_4 = v.inner[1u];
  vec3 bbMin = vec3(uintBitsToFloat(v_2.x), uintBitsToFloat(v_3.y), uintBitsToFloat(v_4.z));
  uvec4 v_5 = v.inner[2u];
  uvec4 v_6 = v.inner[2u];
  uvec4 v_7 = v.inner[2u];
  vec3 bbMax = vec3(uintBitsToFloat(v_5.x), uintBitsToFloat(v_6.y), uintBitsToFloat(v_7.z));
  vec3 bbSize = (bbMax - bbMin);
  float cubeSize = max(max(bbSize.x, bbSize.y), bbSize.z);
  uvec4 v_8 = v.inner[0u];
  float gridSize = float(v_8.y);
  uvec4 v_9 = v.inner[1u];
  float gx = ((gridSize * (position.x - uintBitsToFloat(v_9.x))) / cubeSize);
  uvec4 v_10 = v.inner[1u];
  float gy = ((gridSize * (position.y - uintBitsToFloat(v_10.y))) / cubeSize);
  uvec4 v_11 = v.inner[1u];
  float gz = ((gridSize * (position.z - uintBitsToFloat(v_11.z))) / cubeSize);
  return vec3(gx, gy, gz);
}
uvec3 tint_v3f32_to_v3u32(vec3 value) {
  return uvec3(clamp(value, vec3(0.0f), vec3(4294967040.0f)));
}
uint toIndex1D(uint gridSize, vec3 voxelPos) {
  uvec3 icoord = tint_v3f32_to_v3u32(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}
vec3 loadPosition(uint vertexIndex) {
  uint v_12 = min(((3u * vertexIndex) + 0u), (uint(positions.values.length()) - 1u));
  float v_13 = positions.values[v_12];
  uint v_14 = min(((3u * vertexIndex) + 1u), (uint(positions.values.length()) - 1u));
  float v_15 = positions.values[v_14];
  uint v_16 = min(((3u * vertexIndex) + 2u), (uint(positions.values.length()) - 1u));
  vec3 position = vec3(v_13, v_15, positions.values[v_16]);
  return position;
}
void doIgnore() {
  uvec4 v_17 = v.inner[0u];
  uint g42 = v_17.x;
  uint kj6 = v_1.inner.value1;
  uint v_18 = (uint(counters.values.length()) - 1u);
  uint v_19 = min(uint(0), v_18);
  uint b53 = atomicOr(counters.values[v_19], 0u);
  uint v_20 = (uint(indices.values.length()) - 1u);
  uint v_21 = min(uint(0), v_20);
  uint rwg = indices.values[v_21];
  uint v_22 = (uint(positions.values.length()) - 1u);
  uint v_23 = min(uint(0), v_22);
  float rb5 = positions.values[v_23];
  uint v_24 = (uint(LUT.values.length()) - 1u);
  uint v_25 = min(uint(0), v_24);
  int g55 = atomicOr(LUT.values[v_25], 0);
}
void main_count_inner(uvec3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  uvec4 v_26 = v.inner[0u];
  if ((triangleIndex >= v_26.x)) {
    return;
  }
  doIgnore();
  uint v_27 = ((3u * triangleIndex) + 0u);
  uint v_28 = min(v_27, (uint(indices.values.length()) - 1u));
  uint i0 = indices.values[v_28];
  uint v_29 = ((3u * triangleIndex) + 1u);
  uint v_30 = min(v_29, (uint(indices.values.length()) - 1u));
  uint i1 = indices.values[v_30];
  uint v_31 = ((3u * triangleIndex) + 2u);
  uint v_32 = min(v_31, (uint(indices.values.length()) - 1u));
  uint i2 = indices.values[v_32];
  vec3 p0 = loadPosition(i0);
  vec3 p1 = loadPosition(i1);
  vec3 p2 = loadPosition(i2);
  vec3 center = (((p0 + p1) + p2) / 3.0f);
  vec3 voxelPos = toVoxelPos(center);
  uvec4 v_33 = v.inner[0u];
  uint voxelIndex = toIndex1D(v_33.y, voxelPos);
  uint v_34 = voxelIndex;
  uint v_35 = min(v_34, (uint(counters.values.length()) - 1u));
  uint acefg = atomicAdd(counters.values[v_35], 1u);
  if ((triangleIndex == 0u)) {
    uvec4 v_36 = v.inner[0u];
    v_1.inner.value0 = v_36.y;
    v_1.inner.value_f32_0 = center.x;
    v_1.inner.value_f32_1 = center.y;
    v_1.inner.value_f32_2 = center.z;
  }
}
layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_count_inner(gl_GlobalInvocationID);
}
//
// main_create_lut
//
#version 310 es


struct Dbg {
  uint offsetCounter;
  uint pad0;
  uint pad1;
  uint pad2;
  uint value0;
  uint value1;
  uint value2;
  uint value3;
  float value_f32_0;
  float value_f32_1;
  float value_f32_2;
  float value_f32_3;
};

layout(binding = 0, std140)
uniform uniforms_block_1_ubo {
  uvec4 inner[3];
} v;
layout(binding = 1, std430)
buffer U32s_1_ssbo {
  uint values[];
} indices;
layout(binding = 2, std430)
buffer F32s_1_ssbo {
  float values[];
} positions;
layout(binding = 3, std430)
buffer AU32s_1_ssbo {
  uint values[];
} counters;
layout(binding = 4, std430)
buffer AI32s_1_ssbo {
  int values[];
} LUT;
layout(binding = 5, std430)
buffer dbg_block_1_ssbo {
  Dbg inner;
} v_1;
void doIgnore() {
  uvec4 v_2 = v.inner[0u];
  uint g42 = v_2.x;
  uint kj6 = v_1.inner.value1;
  uint v_3 = (uint(counters.values.length()) - 1u);
  uint v_4 = min(uint(0), v_3);
  uint b53 = atomicOr(counters.values[v_4], 0u);
  uint v_5 = (uint(indices.values.length()) - 1u);
  uint v_6 = min(uint(0), v_5);
  uint rwg = indices.values[v_6];
  uint v_7 = (uint(positions.values.length()) - 1u);
  uint v_8 = min(uint(0), v_7);
  float rb5 = positions.values[v_8];
  uint v_9 = (uint(LUT.values.length()) - 1u);
  uint v_10 = min(uint(0), v_9);
  int g55 = atomicOr(LUT.values[v_10], 0);
}
void main_create_lut_inner(uvec3 GlobalInvocationID) {
  uint voxelIndex = GlobalInvocationID.x;
  doIgnore();
  uvec4 v_11 = v.inner[0u];
  uvec4 v_12 = v.inner[0u];
  uvec4 v_13 = v.inner[0u];
  uint maxVoxels = ((v_11.y * v_12.y) * v_13.y);
  if ((voxelIndex >= maxVoxels)) {
    return;
  }
  uint v_14 = voxelIndex;
  uint v_15 = min(v_14, (uint(counters.values.length()) - 1u));
  uint numTriangles = atomicOr(counters.values[v_15], 0u);
  int offset = -1;
  if ((numTriangles > 0u)) {
    offset = int(atomicAdd(v_1.inner.offsetCounter, numTriangles));
  }
  uint v_16 = voxelIndex;
  uint v_17 = min(v_16, (uint(LUT.values.length()) - 1u));
  atomicExchange(LUT.values[v_17], offset);
}
layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_create_lut_inner(gl_GlobalInvocationID);
}
//
// main_sort_triangles
//
#version 310 es


struct Dbg {
  uint offsetCounter;
  uint pad0;
  uint pad1;
  uint pad2;
  uint value0;
  uint value1;
  uint value2;
  uint value3;
  float value_f32_0;
  float value_f32_1;
  float value_f32_2;
  float value_f32_3;
};

layout(binding = 0, std140)
uniform uniforms_block_1_ubo {
  uvec4 inner[3];
} v;
layout(binding = 1, std430)
buffer U32s_1_ssbo {
  uint values[];
} indices;
layout(binding = 2, std430)
buffer F32s_1_ssbo {
  float values[];
} positions;
layout(binding = 3, std430)
buffer AU32s_1_ssbo {
  uint values[];
} counters;
layout(binding = 4, std430)
buffer AI32s_1_ssbo {
  int values[];
} LUT;
layout(binding = 5, std430)
buffer dbg_block_1_ssbo {
  Dbg inner;
} v_1;
vec3 toVoxelPos(vec3 position) {
  uvec4 v_2 = v.inner[1u];
  uvec4 v_3 = v.inner[1u];
  uvec4 v_4 = v.inner[1u];
  vec3 bbMin = vec3(uintBitsToFloat(v_2.x), uintBitsToFloat(v_3.y), uintBitsToFloat(v_4.z));
  uvec4 v_5 = v.inner[2u];
  uvec4 v_6 = v.inner[2u];
  uvec4 v_7 = v.inner[2u];
  vec3 bbMax = vec3(uintBitsToFloat(v_5.x), uintBitsToFloat(v_6.y), uintBitsToFloat(v_7.z));
  vec3 bbSize = (bbMax - bbMin);
  float cubeSize = max(max(bbSize.x, bbSize.y), bbSize.z);
  uvec4 v_8 = v.inner[0u];
  float gridSize = float(v_8.y);
  uvec4 v_9 = v.inner[1u];
  float gx = ((gridSize * (position.x - uintBitsToFloat(v_9.x))) / cubeSize);
  uvec4 v_10 = v.inner[1u];
  float gy = ((gridSize * (position.y - uintBitsToFloat(v_10.y))) / cubeSize);
  uvec4 v_11 = v.inner[1u];
  float gz = ((gridSize * (position.z - uintBitsToFloat(v_11.z))) / cubeSize);
  return vec3(gx, gy, gz);
}
uvec3 tint_v3f32_to_v3u32(vec3 value) {
  return uvec3(clamp(value, vec3(0.0f), vec3(4294967040.0f)));
}
uint toIndex1D(uint gridSize, vec3 voxelPos) {
  uvec3 icoord = tint_v3f32_to_v3u32(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}
vec3 loadPosition(uint vertexIndex) {
  uint v_12 = min(((3u * vertexIndex) + 0u), (uint(positions.values.length()) - 1u));
  float v_13 = positions.values[v_12];
  uint v_14 = min(((3u * vertexIndex) + 1u), (uint(positions.values.length()) - 1u));
  float v_15 = positions.values[v_14];
  uint v_16 = min(((3u * vertexIndex) + 2u), (uint(positions.values.length()) - 1u));
  vec3 position = vec3(v_13, v_15, positions.values[v_16]);
  return position;
}
void doIgnore() {
  uvec4 v_17 = v.inner[0u];
  uint g42 = v_17.x;
  uint kj6 = v_1.inner.value1;
  uint v_18 = (uint(counters.values.length()) - 1u);
  uint v_19 = min(uint(0), v_18);
  uint b53 = atomicOr(counters.values[v_19], 0u);
  uint v_20 = (uint(indices.values.length()) - 1u);
  uint v_21 = min(uint(0), v_20);
  uint rwg = indices.values[v_21];
  uint v_22 = (uint(positions.values.length()) - 1u);
  uint v_23 = min(uint(0), v_22);
  float rb5 = positions.values[v_23];
  uint v_24 = (uint(LUT.values.length()) - 1u);
  uint v_25 = min(uint(0), v_24);
  int g55 = atomicOr(LUT.values[v_25], 0);
}
void main_sort_triangles_inner(uvec3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  doIgnore();
  uvec4 v_26 = v.inner[0u];
  if ((triangleIndex >= v_26.x)) {
    return;
  }
  uint v_27 = ((3u * triangleIndex) + 0u);
  uint v_28 = min(v_27, (uint(indices.values.length()) - 1u));
  uint i0 = indices.values[v_28];
  uint v_29 = ((3u * triangleIndex) + 1u);
  uint v_30 = min(v_29, (uint(indices.values.length()) - 1u));
  uint i1 = indices.values[v_30];
  uint v_31 = ((3u * triangleIndex) + 2u);
  uint v_32 = min(v_31, (uint(indices.values.length()) - 1u));
  uint i2 = indices.values[v_32];
  vec3 p0 = loadPosition(i0);
  vec3 p1 = loadPosition(i1);
  vec3 p2 = loadPosition(i2);
  vec3 center = (((p0 + p1) + p2) / 3.0f);
  vec3 voxelPos = toVoxelPos(center);
  uvec4 v_33 = v.inner[0u];
  uint voxelIndex = toIndex1D(v_33.y, voxelPos);
  uint v_34 = voxelIndex;
  uint v_35 = min(v_34, (uint(LUT.values.length()) - 1u));
  int triangleOffset = atomicAdd(LUT.values[v_35], 1);
}
layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_sort_triangles_inner(gl_GlobalInvocationID);
}
