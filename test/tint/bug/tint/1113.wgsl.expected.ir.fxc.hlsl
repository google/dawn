struct main_count_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

struct main_create_lut_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

struct main_sort_triangles_inputs {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};


cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[3];
};
RWByteAddressBuffer indices : register(u10);
RWByteAddressBuffer positions : register(u11);
RWByteAddressBuffer counters : register(u20);
RWByteAddressBuffer LUT : register(u21);
RWByteAddressBuffer dbg : register(u50);
float3 toVoxelPos(float3 position) {
  float v = asfloat(uniforms[1u].x);
  float v_1 = asfloat(uniforms[1u].y);
  float3 bbMin = float3(v, v_1, asfloat(uniforms[1u].z));
  float v_2 = asfloat(uniforms[2u].x);
  float v_3 = asfloat(uniforms[2u].y);
  float3 bbMax = float3(v_2, v_3, asfloat(uniforms[2u].z));
  float3 bbSize = (bbMax - bbMin);
  float v_4 = max(bbSize.x, bbSize.y);
  float cubeSize = max(v_4, bbSize.z);
  float gridSize = float(uniforms[0u].y);
  float v_5 = gridSize;
  float v_6 = (v_5 * (position[0u] - asfloat(uniforms[1u].x)));
  float gx = (v_6 / cubeSize);
  float v_7 = gridSize;
  float v_8 = (v_7 * (position[1u] - asfloat(uniforms[1u].y)));
  float gy = (v_8 / cubeSize);
  float v_9 = gridSize;
  float v_10 = (v_9 * (position[2u] - asfloat(uniforms[1u].z)));
  float gz = (v_10 / cubeSize);
  return float3(gx, gy, gz);
}

uint3 tint_v3f32_to_v3u32(float3 value) {
  return (((value <= (4294967040.0f).xxx)) ? ((((value >= (0.0f).xxx)) ? (uint3(value)) : ((0u).xxx))) : ((4294967295u).xxx));
}

uint toIndex1D(uint gridSize, float3 voxelPos) {
  uint3 icoord = tint_v3f32_to_v3u32(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}

uint tint_mod_u32(uint lhs, uint rhs) {
  uint v_11 = (((rhs == 0u)) ? (1u) : (rhs));
  return (lhs - ((lhs / v_11) * v_11));
}

uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / (((rhs == 0u)) ? (1u) : (rhs)));
}

uint3 toIndex3D(uint gridSize, uint index) {
  uint z = tint_div_u32(index, (gridSize * gridSize));
  uint y = tint_div_u32((index - ((gridSize * gridSize) * z)), gridSize);
  uint x = tint_mod_u32(index, gridSize);
  return uint3(x, y, z);
}

float3 loadPosition(uint vertexIndex) {
  float v_12 = asfloat(positions.Load((0u + (uint(((3u * vertexIndex) + 0u)) * 4u))));
  float v_13 = asfloat(positions.Load((0u + (uint(((3u * vertexIndex) + 1u)) * 4u))));
  float3 position = float3(v_12, v_13, asfloat(positions.Load((0u + (uint(((3u * vertexIndex) + 2u)) * 4u)))));
  return position;
}

void doIgnore() {
  uint g42 = uniforms[0u].x;
  uint kj6 = dbg.Load(20u);
  uint v_14 = 0u;
  counters.InterlockedOr(uint(0u), 0u, v_14);
  uint b53 = v_14;
  uint rwg = indices.Load(0u);
  float rb5 = asfloat(positions.Load(0u));
  int v_15 = int(0);
  LUT.InterlockedOr(int(0u), int(0), v_15);
  int g55 = v_15;
}

void main_count_inner(uint3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID[0u];
  if ((triangleIndex >= uniforms[0u].x)) {
    return;
  }
  doIgnore();
  uint i0 = indices.Load((0u + (uint(((3u * triangleIndex) + 0u)) * 4u)));
  uint i1 = indices.Load((0u + (uint(((3u * triangleIndex) + 1u)) * 4u)));
  uint i2 = indices.Load((0u + (uint(((3u * triangleIndex) + 2u)) * 4u)));
  float3 p0 = loadPosition(i0);
  float3 p1 = loadPosition(i1);
  float3 p2 = loadPosition(i2);
  float3 center = (((p0 + p1) + p2) / 3.0f);
  float3 voxelPos = toVoxelPos(center);
  uint voxelIndex = toIndex1D(uniforms[0u].y, voxelPos);
  uint v_16 = (uint(voxelIndex) * 4u);
  uint v_17 = 0u;
  counters.InterlockedAdd(uint(0u), 1u, v_17);
  uint acefg = v_17;
  if ((triangleIndex == 0u)) {
    dbg.Store(16u, uniforms[0u].y);
    dbg.Store(32u, asuint(center.x));
    dbg.Store(36u, asuint(center.y));
    dbg.Store(40u, asuint(center.z));
  }
}

void main_create_lut_inner(uint3 GlobalInvocationID) {
  uint voxelIndex = GlobalInvocationID[0u];
  doIgnore();
  uint maxVoxels = ((uniforms[0u].y * uniforms[0u].y) * uniforms[0u].y);
  if ((voxelIndex >= maxVoxels)) {
    return;
  }
  uint v_18 = (uint(voxelIndex) * 4u);
  uint v_19 = 0u;
  counters.InterlockedOr(uint(0u), 0u, v_19);
  uint numTriangles = v_19;
  int offset = int(-1);
  if ((numTriangles > 0u)) {
    uint v_20 = numTriangles;
    uint v_21 = 0u;
    dbg.InterlockedAdd(uint(0u), v_20, v_21);
    offset = int(v_21);
  }
  uint v_22 = (uint(voxelIndex) * 4u);
  int v_23 = offset;
  int v_24 = int(0);
  LUT.InterlockedExchange(int(0u), v_23, v_24);
}

void main_sort_triangles_inner(uint3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID[0u];
  doIgnore();
  if ((triangleIndex >= uniforms[0u].x)) {
    return;
  }
  uint i0 = indices.Load((0u + (uint(((3u * triangleIndex) + 0u)) * 4u)));
  uint i1 = indices.Load((0u + (uint(((3u * triangleIndex) + 1u)) * 4u)));
  uint i2 = indices.Load((0u + (uint(((3u * triangleIndex) + 2u)) * 4u)));
  float3 p0 = loadPosition(i0);
  float3 p1 = loadPosition(i1);
  float3 p2 = loadPosition(i2);
  float3 center = (((p0 + p1) + p2) / 3.0f);
  float3 voxelPos = toVoxelPos(center);
  uint voxelIndex = toIndex1D(uniforms[0u].y, voxelPos);
  uint v_25 = (uint(voxelIndex) * 4u);
  int v_26 = int(0);
  LUT.InterlockedAdd(int(0u), int(1), v_26);
  int triangleOffset = v_26;
}

[numthreads(128, 1, 1)]
void main_count(main_count_inputs inputs) {
  main_count_inner(inputs.GlobalInvocationID);
}

[numthreads(128, 1, 1)]
void main_create_lut(main_create_lut_inputs inputs) {
  main_create_lut_inner(inputs.GlobalInvocationID);
}

[numthreads(128, 1, 1)]
void main_sort_triangles(main_sort_triangles_inputs inputs) {
  main_sort_triangles_inner(inputs.GlobalInvocationID);
}

