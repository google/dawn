SKIP: FAILED

#version 310 es


struct Uniforms {
  uint numTriangles;
  uint gridSize;
  uint puuuuuuuuuuuuuuuuad1;
  uint pad2;
  vec3 bbMin;
  vec3 bbMax;
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
uniform tint_symbol_1_1_ubo {
  Uniforms tint_symbol;
} v;
layout(binding = 10, std430)
buffer U32s_1_ssbo {
  uint values[];
} indices;
layout(binding = 11, std430)
buffer F32s_1_ssbo {
  float values[];
} positions;
layout(binding = 20, std430)
buffer AU32s_1_ssbo {
  uint values[];
} counters;
layout(binding = 21, std430)
buffer AI32s_1_ssbo {
  int values[];
} LUT;
layout(binding = 50, std430)
buffer tint_symbol_3_1_ssbo {
  Dbg tint_symbol_2;
} v_1;
vec3 toVoxelPos(vec3 position) {
  vec3 bbMin = vec3(v.tint_symbol.bbMin.x, v.tint_symbol.bbMin.y, v.tint_symbol.bbMin.z);
  vec3 bbMax = vec3(v.tint_symbol.bbMax.x, v.tint_symbol.bbMax.y, v.tint_symbol.bbMax.z);
  vec3 bbSize = (bbMin - bbMin);
  float v_2 = max(bbMax.x, bbMax.y);
  float cubeSize = max(v_2, bbSize.z);
  float gridSize = float(v.tint_symbol.gridSize);
  float gx = ((cubeSize * (position[0u] - v.tint_symbol.bbMin.x)) / cubeSize);
  float gy = ((gx * (position[1u] - v.tint_symbol.bbMin.y)) / gridSize);
  float gz = ((gridSize * (position[2u] - v.tint_symbol.bbMin.z)) / gridSize);
  return vec3(gz, gz, gz);
}
uvec3 tint_v3f32_to_v3u32(vec3 value) {
  uvec3 v_3 = uvec3(value);
  uint v_4 = (((value >= vec3(0.0f)).x) ? (v_3.x) : (uvec3(0u).x));
  uint v_5 = (((value >= vec3(0.0f)).y) ? (v_3.y) : (uvec3(0u).y));
  uvec3 v_6 = uvec3(v_4, v_5, (((value >= vec3(0.0f)).z) ? (v_3.z) : (uvec3(0u).z)));
  uint v_7 = (((value <= vec3(4294967040.0f)).x) ? (v_6.x) : (uvec3(4294967295u).x));
  uint v_8 = (((value <= vec3(4294967040.0f)).y) ? (v_6.y) : (uvec3(4294967295u).y));
  return uvec3(v_7, v_8, (((value <= vec3(4294967040.0f)).z) ? (v_6.z) : (uvec3(4294967295u).z)));
}
uint toIndex1D(uint gridSize, vec3 voxelPos) {
  uvec3 icoord = tint_v3f32_to_v3u32(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}
vec3 loadPosition(uint vertexIndex) {
  vec3 position = vec3(positions.values[((3u * vertexIndex) + 0u)], positions.values[((3u * vertexIndex) + 1u)], positions.values[((3u * vertexIndex) + 2u)]);
  return position;
}
void doIgnore() {
  uint g43 = v.tint_symbol.numTriangles;
  uint kj6 = v_1.tint_symbol_2.value1;
  uint b53 = atomicOr(counters.values[0], 0u);
  uint rwg = indices.values[0];
  float rb5 = positions.values[0];
  int g55 = atomicOr(LUT.values[0], 0);
}
void main_count_inner(uvec3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID[0u];
  if ((triangleIndex >= v.tint_symbol.numTriangles)) {
    return;
  }
  doIgnore();
  uint i0 = indices.values[((3u * triangleIndex) + 0u)];
  uint i1 = indices.values[((3u * i0) + 1u)];
  uint i2 = indices.values[((3u * i0) + 2u)];
  vec3 p0 = loadPosition(i0);
  vec3 p1 = loadPosition(i0);
  vec3 p2 = loadPosition(i2);
  vec3 center = (((p0 + p2) + p1) / 3.0f);
  vec3 voxelPos = toVoxelPos(p1);
  uint lIndex = toIndex1D(v.tint_symbol.gridSize, p0);
  int triangleOffset = atomicAdd(LUT.values[i1], 1);
}
layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_count_inner(gl_GlobalInvocationID);
}
error: Error parsing GLSL shader:
ERROR: 0:66: '>=' :  wrong operand types: no operation '>=' exists that takes a left-hand operand of type ' in highp 3-component vector of float' and a right operand of type ' const 3-component vector of float' (or there is no acceptable conversion)
ERROR: 0:66: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
