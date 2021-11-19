#version 310 es
precision mediump float;


layout (binding = 0) uniform Uniforms_1 {
  uint numTriangles;
  uint gridSize;
  uint pad1;
  uint pad2;
  vec3 bbMin;
  vec3 bbMax;
} uniforms;
layout (binding = 10) buffer U32s_1 {
  uint values[];
} indices;
layout (binding = 11) buffer F32s_1 {
  float values[];
} positions;
layout (binding = 20) buffer AU32s_1 {
  uint values[];
} counters;
layout (binding = 21) buffer AI32s_1 {
  int values[];
} LUT;
layout (binding = 50) buffer Dbg_1 {
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
} dbg;

vec3 toVoxelPos(vec3 position) {
  vec3 bbMin = vec3(uniforms.bbMin.x, uniforms.bbMin.y, uniforms.bbMin.z);
  vec3 bbMax = vec3(uniforms.bbMax.x, uniforms.bbMax.y, uniforms.bbMax.z);
  vec3 bbSize = (bbMax - bbMin);
  float cubeSize = max(max(bbSize.x, bbSize.y), bbSize.z);
  float gridSize = float(uniforms.gridSize);
  float gx = ((gridSize * (position.x - uniforms.bbMin.x)) / cubeSize);
  float gy = ((gridSize * (position.y - uniforms.bbMin.y)) / cubeSize);
  float gz = ((gridSize * (position.z - uniforms.bbMin.z)) / cubeSize);
  return vec3(gx, gy, gz);
}

uint toIndex1D(uint gridSize, vec3 voxelPos) {
  uvec3 icoord = uvec3(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}

vec3 loadPosition(uint vertexIndex) {
  vec3 position = vec3(positions.values[((3u * vertexIndex) + 0u)], positions.values[((3u * vertexIndex) + 1u)], positions.values[((3u * vertexIndex) + 2u)]);
  return position;
}

void doIgnore() {
  uint g42 = uniforms.numTriangles;
  uint kj6 = dbg.value1;
  uint b53 = atomicOr(counters.values[0], 0u);
  uint rwg = indices.values[0];
  float rb5 = positions.values[0];
  int g55 = atomicOr(LUT.values[0], 0);
}

struct tint_symbol_1 {
  uvec3 GlobalInvocationID;
};

void main_count_inner(uvec3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  if ((triangleIndex >= uniforms.numTriangles)) {
    return;
  }
  doIgnore();
  uint i0 = indices.values[((3u * triangleIndex) + 0u)];
  uint i1 = indices.values[((3u * triangleIndex) + 1u)];
  uint i2 = indices.values[((3u * triangleIndex) + 2u)];
  vec3 p0 = loadPosition(i0);
  vec3 p1 = loadPosition(i1);
  vec3 p2 = loadPosition(i2);
  vec3 center = (((p0 + p1) + p2) / 3.0f);
  vec3 voxelPos = toVoxelPos(center);
  uint voxelIndex = toIndex1D(uniforms.gridSize, voxelPos);
  uint acefg = atomicAdd(counters.values[voxelIndex], 1u);
  if ((triangleIndex == 0u)) {
    dbg.value0 = uniforms.gridSize;
    dbg.value_f32_0 = center.x;
    dbg.value_f32_1 = center.y;
    dbg.value_f32_2 = center.z;
  }
}

struct tint_symbol_3 {
  uvec3 GlobalInvocationID;
};
struct tint_symbol_5 {
  uvec3 GlobalInvocationID;
};

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main_count(tint_symbol_1 tint_symbol) {
  main_count_inner(tint_symbol.GlobalInvocationID);
  return;
}
void main() {
  tint_symbol_1 inputs;
  inputs.GlobalInvocationID = gl_GlobalInvocationID;
  main_count(inputs);
}


#version 310 es
precision mediump float;


layout (binding = 0) uniform Uniforms_1 {
  uint numTriangles;
  uint gridSize;
  uint pad1;
  uint pad2;
  vec3 bbMin;
  vec3 bbMax;
} uniforms;
layout (binding = 10) buffer U32s_1 {
  uint values[];
} indices;
layout (binding = 11) buffer F32s_1 {
  float values[];
} positions;
layout (binding = 20) buffer AU32s_1 {
  uint values[];
} counters;
layout (binding = 21) buffer AI32s_1 {
  int values[];
} LUT;
layout (binding = 50) buffer Dbg_1 {
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
} dbg;

void doIgnore() {
  uint g42 = uniforms.numTriangles;
  uint kj6 = dbg.value1;
  uint b53 = atomicOr(counters.values[0], 0u);
  uint rwg = indices.values[0];
  float rb5 = positions.values[0];
  int g55 = atomicOr(LUT.values[0], 0);
}

struct tint_symbol_1 {
  uvec3 GlobalInvocationID;
};
struct tint_symbol_3 {
  uvec3 GlobalInvocationID;
};

void main_create_lut_inner(uvec3 GlobalInvocationID) {
  uint voxelIndex = GlobalInvocationID.x;
  doIgnore();
  uint maxVoxels = ((uniforms.gridSize * uniforms.gridSize) * uniforms.gridSize);
  if ((voxelIndex >= maxVoxels)) {
    return;
  }
  uint numTriangles = atomicOr(counters.values[voxelIndex], 0u);
  int offset = -1;
  if ((numTriangles > 0u)) {
    offset = int(atomicAdd(dbg.offsetCounter, numTriangles));
  }
  atomicExchange(LUT.values[voxelIndex], offset);
}

struct tint_symbol_5 {
  uvec3 GlobalInvocationID;
};

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main_create_lut(tint_symbol_3 tint_symbol_2) {
  main_create_lut_inner(tint_symbol_2.GlobalInvocationID);
  return;
}
void main() {
  tint_symbol_3 inputs;
  inputs.GlobalInvocationID = gl_GlobalInvocationID;
  main_create_lut(inputs);
}


#version 310 es
precision mediump float;


layout (binding = 0) uniform Uniforms_1 {
  uint numTriangles;
  uint gridSize;
  uint pad1;
  uint pad2;
  vec3 bbMin;
  vec3 bbMax;
} uniforms;
layout (binding = 10) buffer U32s_1 {
  uint values[];
} indices;
layout (binding = 11) buffer F32s_1 {
  float values[];
} positions;
layout (binding = 20) buffer AU32s_1 {
  uint values[];
} counters;
layout (binding = 21) buffer AI32s_1 {
  int values[];
} LUT;
layout (binding = 50) buffer Dbg_1 {
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
} dbg;

vec3 toVoxelPos(vec3 position) {
  vec3 bbMin = vec3(uniforms.bbMin.x, uniforms.bbMin.y, uniforms.bbMin.z);
  vec3 bbMax = vec3(uniforms.bbMax.x, uniforms.bbMax.y, uniforms.bbMax.z);
  vec3 bbSize = (bbMax - bbMin);
  float cubeSize = max(max(bbSize.x, bbSize.y), bbSize.z);
  float gridSize = float(uniforms.gridSize);
  float gx = ((gridSize * (position.x - uniforms.bbMin.x)) / cubeSize);
  float gy = ((gridSize * (position.y - uniforms.bbMin.y)) / cubeSize);
  float gz = ((gridSize * (position.z - uniforms.bbMin.z)) / cubeSize);
  return vec3(gx, gy, gz);
}

uint toIndex1D(uint gridSize, vec3 voxelPos) {
  uvec3 icoord = uvec3(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}

vec3 loadPosition(uint vertexIndex) {
  vec3 position = vec3(positions.values[((3u * vertexIndex) + 0u)], positions.values[((3u * vertexIndex) + 1u)], positions.values[((3u * vertexIndex) + 2u)]);
  return position;
}

void doIgnore() {
  uint g42 = uniforms.numTriangles;
  uint kj6 = dbg.value1;
  uint b53 = atomicOr(counters.values[0], 0u);
  uint rwg = indices.values[0];
  float rb5 = positions.values[0];
  int g55 = atomicOr(LUT.values[0], 0);
}

struct tint_symbol_1 {
  uvec3 GlobalInvocationID;
};
struct tint_symbol_3 {
  uvec3 GlobalInvocationID;
};
struct tint_symbol_5 {
  uvec3 GlobalInvocationID;
};

void main_sort_triangles_inner(uvec3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  doIgnore();
  if ((triangleIndex >= uniforms.numTriangles)) {
    return;
  }
  uint i0 = indices.values[((3u * triangleIndex) + 0u)];
  uint i1 = indices.values[((3u * triangleIndex) + 1u)];
  uint i2 = indices.values[((3u * triangleIndex) + 2u)];
  vec3 p0 = loadPosition(i0);
  vec3 p1 = loadPosition(i1);
  vec3 p2 = loadPosition(i2);
  vec3 center = (((p0 + p1) + p2) / 3.0f);
  vec3 voxelPos = toVoxelPos(center);
  uint voxelIndex = toIndex1D(uniforms.gridSize, voxelPos);
  int triangleOffset = atomicAdd(LUT.values[voxelIndex], 1);
}

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main_sort_triangles(tint_symbol_5 tint_symbol_4) {
  main_sort_triangles_inner(tint_symbol_4.GlobalInvocationID);
  return;
}
void main() {
  tint_symbol_5 inputs;
  inputs.GlobalInvocationID = gl_GlobalInvocationID;
  main_sort_triangles(inputs);
}


