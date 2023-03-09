#version 310 es

uvec3 tint_select(uvec3 param_0, uvec3 param_1, bvec3 param_2) {
    return uvec3(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2]);
}


uvec3 tint_ftou(vec3 v) {
  return tint_select(uvec3(4294967295u), tint_select(uvec3(v), uvec3(0u), lessThan(v, vec3(0.0f))), lessThan(v, vec3(4294967040.0f)));
}

struct Uniforms {
  uint numTriangles;
  uint gridSize;
  uint puuuuuuuuuuuuuuuuad1;
  uint pad2;
  vec3 bbMin;
  uint pad;
  vec3 bbMax;
  uint pad_1;
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

layout(binding = 0, std140) uniform uniforms_block_ubo {
  Uniforms inner;
} uniforms;

layout(binding = 10, std430) buffer U32s_ssbo {
  uint values[];
} indices;

layout(binding = 11, std430) buffer F32s_ssbo {
  float values[];
} positions;

layout(binding = 20, std430) buffer AU32s_ssbo {
  uint values[];
} counters;

layout(binding = 21, std430) buffer AI32s_ssbo {
  int values[];
} LUT;

layout(binding = 50, std430) buffer dbg_block_ssbo {
  Dbg inner;
} dbg;

vec3 toVoxelPos(vec3 position) {
  vec3 bbMin = vec3(uniforms.inner.bbMin.x, uniforms.inner.bbMin.y, uniforms.inner.bbMin.z);
  vec3 bbMax = vec3(uniforms.inner.bbMax.x, uniforms.inner.bbMax.y, uniforms.inner.bbMax.z);
  vec3 bbSize = (bbMin - bbMin);
  float cubeSize = max(max(bbMax.x, bbMax.y), bbSize.z);
  float gridSize = float(uniforms.inner.gridSize);
  float gx = ((cubeSize * (position.x - uniforms.inner.bbMin.x)) / cubeSize);
  float gy = ((gx * (position.y - uniforms.inner.bbMin.y)) / gridSize);
  float gz = ((gridSize * (position.z - uniforms.inner.bbMin.z)) / gridSize);
  return vec3(gz, gz, gz);
}

uint toIndex1D(uint gridSize, vec3 voxelPos) {
  uvec3 icoord = tint_ftou(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}

vec3 loadPosition(uint vertexIndex) {
  vec3 position = vec3(positions.values[((3u * vertexIndex) + 0u)], positions.values[((3u * vertexIndex) + 1u)], positions.values[((3u * vertexIndex) + 2u)]);
  return position;
}

void doIgnore() {
  uint g43 = uniforms.inner.numTriangles;
  uint kj6 = dbg.inner.value1;
  uint b53 = atomicOr(counters.values[0], 0u);
  uint rwg = indices.values[0];
  float rb5 = positions.values[0];
  int g55 = atomicOr(LUT.values[0], 0);
}

void main_count(uvec3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  if ((triangleIndex >= uniforms.inner.numTriangles)) {
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
  uint lIndex = toIndex1D(uniforms.inner.gridSize, p0);
  int triangleOffset = atomicAdd(LUT.values[i1], 1);
}

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_count(gl_GlobalInvocationID);
  return;
}
