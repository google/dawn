#version 310 es


struct Uniforms {
  uint numTriangles;
  uint gridSize;
  uint puuuuuuuuuuuuuuuuad1;
  uint pad2;
  vec3 bbMin;
  uint tint_pad_0;
  vec3 bbMax;
  uint tint_pad_1;
};

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
  Uniforms inner;
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
  vec3 v_2 = v.inner.bbMin;
  vec3 v_3 = v.inner.bbMin;
  vec3 v_4 = v.inner.bbMin;
  vec3 bbMin = vec3(v_2.x, v_3.y, v_4.z);
  vec3 v_5 = v.inner.bbMax;
  vec3 v_6 = v.inner.bbMax;
  vec3 v_7 = v.inner.bbMax;
  vec3 bbMax = vec3(v_5.x, v_6.y, v_7.z);
  vec3 bbSize = (bbMin - bbMin);
  float cubeSize = max(max(bbMax.x, bbMax.y), bbSize.z);
  float gridSize = float(v.inner.gridSize);
  vec3 v_8 = v.inner.bbMin;
  float gx = ((cubeSize * (position.x - v_8.x)) / cubeSize);
  vec3 v_9 = v.inner.bbMin;
  float gy = ((gx * (position.y - v_9.y)) / gridSize);
  vec3 v_10 = v.inner.bbMin;
  float gz = ((gridSize * (position.z - v_10.z)) / gridSize);
  return vec3(gz, gz, gz);
}
uvec3 tint_v3f32_to_v3u32(vec3 value) {
  return uvec3(clamp(value, vec3(0.0f), vec3(4294967040.0f)));
}
uint toIndex1D(uint gridSize, vec3 voxelPos) {
  uvec3 icoord = tint_v3f32_to_v3u32(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}
vec3 loadPosition(uint vertexIndex) {
  uint v_11 = min(((3u * vertexIndex) + 0u), (uint(positions.values.length()) - 1u));
  float v_12 = positions.values[v_11];
  uint v_13 = min(((3u * vertexIndex) + 1u), (uint(positions.values.length()) - 1u));
  float v_14 = positions.values[v_13];
  uint v_15 = min(((3u * vertexIndex) + 2u), (uint(positions.values.length()) - 1u));
  vec3 position = vec3(v_12, v_14, positions.values[v_15]);
  return position;
}
void doIgnore() {
  uint g43 = v.inner.numTriangles;
  uint kj6 = v_1.inner.value1;
  uint v_16 = (uint(counters.values.length()) - 1u);
  uint v_17 = min(uint(0), v_16);
  uint b53 = atomicOr(counters.values[v_17], 0u);
  uint v_18 = (uint(indices.values.length()) - 1u);
  uint v_19 = min(uint(0), v_18);
  uint rwg = indices.values[v_19];
  uint v_20 = (uint(positions.values.length()) - 1u);
  uint v_21 = min(uint(0), v_20);
  float rb5 = positions.values[v_21];
  uint v_22 = (uint(LUT.values.length()) - 1u);
  uint v_23 = min(uint(0), v_22);
  int g55 = atomicOr(LUT.values[v_23], 0);
}
void main_count_inner(uvec3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  if ((triangleIndex >= v.inner.numTriangles)) {
    return;
  }
  doIgnore();
  uint v_24 = ((3u * triangleIndex) + 0u);
  uint v_25 = min(v_24, (uint(indices.values.length()) - 1u));
  uint i0 = indices.values[v_25];
  uint v_26 = ((3u * i0) + 1u);
  uint v_27 = min(v_26, (uint(indices.values.length()) - 1u));
  uint i1 = indices.values[v_27];
  uint v_28 = ((3u * i0) + 2u);
  uint v_29 = min(v_28, (uint(indices.values.length()) - 1u));
  uint i2 = indices.values[v_29];
  vec3 p0 = loadPosition(i0);
  vec3 p1 = loadPosition(i0);
  vec3 p2 = loadPosition(i2);
  vec3 center = (((p0 + p2) + p1) / 3.0f);
  vec3 voxelPos = toVoxelPos(p1);
  uint lIndex = toIndex1D(v.inner.gridSize, p0);
  uint v_30 = i1;
  uint v_31 = min(v_30, (uint(LUT.values.length()) - 1u));
  int triangleOffset = atomicAdd(LUT.values[v_31], 1);
}
layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_count_inner(gl_GlobalInvocationID);
}
