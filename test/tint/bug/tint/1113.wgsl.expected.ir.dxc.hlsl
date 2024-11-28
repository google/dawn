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
  float3 bbMin = float3(asfloat(uniforms[1u].x), asfloat(uniforms[1u].y), asfloat(uniforms[1u].z));
  float3 bbMax = float3(asfloat(uniforms[2u].x), asfloat(uniforms[2u].y), asfloat(uniforms[2u].z));
  float3 bbSize = (bbMax - bbMin);
  float cubeSize = max(max(bbSize.x, bbSize.y), bbSize.z);
  float gridSize = float(uniforms[0u].y);
  float gx = ((gridSize * (position.x - asfloat(uniforms[1u].x))) / cubeSize);
  float gy = ((gridSize * (position.y - asfloat(uniforms[1u].y))) / cubeSize);
  float gz = ((gridSize * (position.z - asfloat(uniforms[1u].z))) / cubeSize);
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
  uint v = (((rhs == 0u)) ? (1u) : (rhs));
  return (lhs - ((lhs / v) * v));
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
  uint v_1 = 0u;
  positions.GetDimensions(v_1);
  uint v_2 = 0u;
  positions.GetDimensions(v_2);
  uint v_3 = 0u;
  positions.GetDimensions(v_3);
  float3 position = float3(asfloat(positions.Load((0u + (min(((3u * vertexIndex) + 0u), ((v_1 / 4u) - 1u)) * 4u)))), asfloat(positions.Load((0u + (min(((3u * vertexIndex) + 1u), ((v_2 / 4u) - 1u)) * 4u)))), asfloat(positions.Load((0u + (min(((3u * vertexIndex) + 2u), ((v_3 / 4u) - 1u)) * 4u)))));
  return position;
}

void doIgnore() {
  uint g42 = uniforms[0u].x;
  uint kj6 = dbg.Load(20u);
  uint v_4 = 0u;
  counters.GetDimensions(v_4);
  uint v_5 = ((v_4 / 4u) - 1u);
  uint v_6 = 0u;
  counters.InterlockedOr(uint((0u + (min(uint(int(0)), v_5) * 4u))), 0u, v_6);
  uint b53 = v_6;
  uint v_7 = 0u;
  indices.GetDimensions(v_7);
  uint v_8 = ((v_7 / 4u) - 1u);
  uint rwg = indices.Load((0u + (min(uint(int(0)), v_8) * 4u)));
  uint v_9 = 0u;
  positions.GetDimensions(v_9);
  uint v_10 = ((v_9 / 4u) - 1u);
  float rb5 = asfloat(positions.Load((0u + (min(uint(int(0)), v_10) * 4u))));
  uint v_11 = 0u;
  LUT.GetDimensions(v_11);
  uint v_12 = ((v_11 / 4u) - 1u);
  int v_13 = int(0);
  LUT.InterlockedOr(int((0u + (min(uint(int(0)), v_12) * 4u))), int(0), v_13);
  int g55 = v_13;
}

void main_count_inner(uint3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  if ((triangleIndex >= uniforms[0u].x)) {
    return;
  }
  doIgnore();
  uint v_14 = 0u;
  indices.GetDimensions(v_14);
  uint i0 = indices.Load((0u + (min(((3u * triangleIndex) + 0u), ((v_14 / 4u) - 1u)) * 4u)));
  uint v_15 = 0u;
  indices.GetDimensions(v_15);
  uint i1 = indices.Load((0u + (min(((3u * triangleIndex) + 1u), ((v_15 / 4u) - 1u)) * 4u)));
  uint v_16 = 0u;
  indices.GetDimensions(v_16);
  uint i2 = indices.Load((0u + (min(((3u * triangleIndex) + 2u), ((v_16 / 4u) - 1u)) * 4u)));
  float3 p0 = loadPosition(i0);
  float3 p1 = loadPosition(i1);
  float3 p2 = loadPosition(i2);
  float3 center = (((p0 + p1) + p2) / 3.0f);
  float3 voxelPos = toVoxelPos(center);
  uint voxelIndex = toIndex1D(uniforms[0u].y, voxelPos);
  uint v_17 = 0u;
  counters.GetDimensions(v_17);
  uint v_18 = 0u;
  counters.InterlockedAdd(uint((0u + (min(voxelIndex, ((v_17 / 4u) - 1u)) * 4u))), 1u, v_18);
  uint acefg = v_18;
  if ((triangleIndex == 0u)) {
    dbg.Store(16u, uniforms[0u].y);
    dbg.Store(32u, asuint(center.x));
    dbg.Store(36u, asuint(center.y));
    dbg.Store(40u, asuint(center.z));
  }
}

void main_create_lut_inner(uint3 GlobalInvocationID) {
  uint voxelIndex = GlobalInvocationID.x;
  doIgnore();
  uint maxVoxels = ((uniforms[0u].y * uniforms[0u].y) * uniforms[0u].y);
  if ((voxelIndex >= maxVoxels)) {
    return;
  }
  uint v_19 = 0u;
  counters.GetDimensions(v_19);
  uint v_20 = 0u;
  counters.InterlockedOr(uint((0u + (min(voxelIndex, ((v_19 / 4u) - 1u)) * 4u))), 0u, v_20);
  uint numTriangles = v_20;
  int offset = int(-1);
  if ((numTriangles > 0u)) {
    uint v_21 = numTriangles;
    uint v_22 = 0u;
    dbg.InterlockedAdd(uint(0u), v_21, v_22);
    offset = int(v_22);
  }
  uint v_23 = 0u;
  LUT.GetDimensions(v_23);
  int v_24 = offset;
  int v_25 = int(0);
  LUT.InterlockedExchange(int((0u + (min(voxelIndex, ((v_23 / 4u) - 1u)) * 4u))), v_24, v_25);
}

void main_sort_triangles_inner(uint3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  doIgnore();
  if ((triangleIndex >= uniforms[0u].x)) {
    return;
  }
  uint v_26 = 0u;
  indices.GetDimensions(v_26);
  uint i0 = indices.Load((0u + (min(((3u * triangleIndex) + 0u), ((v_26 / 4u) - 1u)) * 4u)));
  uint v_27 = 0u;
  indices.GetDimensions(v_27);
  uint i1 = indices.Load((0u + (min(((3u * triangleIndex) + 1u), ((v_27 / 4u) - 1u)) * 4u)));
  uint v_28 = 0u;
  indices.GetDimensions(v_28);
  uint i2 = indices.Load((0u + (min(((3u * triangleIndex) + 2u), ((v_28 / 4u) - 1u)) * 4u)));
  float3 p0 = loadPosition(i0);
  float3 p1 = loadPosition(i1);
  float3 p2 = loadPosition(i2);
  float3 center = (((p0 + p1) + p2) / 3.0f);
  float3 voxelPos = toVoxelPos(center);
  uint voxelIndex = toIndex1D(uniforms[0u].y, voxelPos);
  uint v_29 = 0u;
  LUT.GetDimensions(v_29);
  int v_30 = int(0);
  LUT.InterlockedAdd(int((0u + (min(voxelIndex, ((v_29 / 4u) - 1u)) * 4u))), int(1), v_30);
  int triangleOffset = v_30;
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

