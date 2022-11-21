cbuffer cbuffer_uniforms : register(b0, space0) {
  uint4 uniforms[3];
};
RWByteAddressBuffer indices : register(u10, space0);
RWByteAddressBuffer positions : register(u11, space0);
RWByteAddressBuffer counters : register(u20, space0);
RWByteAddressBuffer LUT : register(u21, space0);
RWByteAddressBuffer dbg : register(u50, space0);

float3 toVoxelPos(float3 position) {
  float3 bbMin = float3(asfloat(uniforms[1].x), asfloat(uniforms[1].y), asfloat(uniforms[1].z));
  float3 bbMax = float3(asfloat(uniforms[2].x), asfloat(uniforms[2].y), asfloat(uniforms[2].z));
  float3 bbSize = (bbMax - bbMin);
  float cubeSize = max(max(bbSize.x, bbSize.y), bbSize.z);
  float gridSize = float(uniforms[0].y);
  float gx = ((gridSize * (position.x - asfloat(uniforms[1].x))) / cubeSize);
  float gy = ((gridSize * (position.y - asfloat(uniforms[1].y))) / cubeSize);
  float gz = ((gridSize * (position.z - asfloat(uniforms[1].z))) / cubeSize);
  return float3(gx, gy, gz);
}

uint toIndex1D(uint gridSize, float3 voxelPos) {
  uint3 icoord = uint3(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}

uint tint_div(uint lhs, uint rhs) {
  return (lhs / ((rhs == 0u) ? 1u : rhs));
}

uint tint_mod(uint lhs, uint rhs) {
  return (lhs % ((rhs == 0u) ? 1u : rhs));
}

uint3 toIndex3D(uint gridSize, uint index) {
  uint z_1 = tint_div(index, (gridSize * gridSize));
  uint y_1 = tint_div((index - ((gridSize * gridSize) * z_1)), gridSize);
  uint x_1 = tint_mod(index, gridSize);
  return uint3(x_1, y_1, z_1);
}

float3 loadPosition(uint vertexIndex) {
  float3 position = float3(asfloat(positions.Load((4u * ((3u * vertexIndex) + 0u)))), asfloat(positions.Load((4u * ((3u * vertexIndex) + 1u)))), asfloat(positions.Load((4u * ((3u * vertexIndex) + 2u)))));
  return position;
}

uint tint_atomicLoad(RWByteAddressBuffer buffer, uint offset) {
  uint value = 0;
  buffer.InterlockedOr(offset, 0, value);
  return value;
}


int tint_atomicLoad_1(RWByteAddressBuffer buffer, uint offset) {
  int value = 0;
  buffer.InterlockedOr(offset, 0, value);
  return value;
}


void doIgnore() {
  uint g42 = uniforms[0].x;
  uint kj6 = dbg.Load(20u);
  uint b53 = tint_atomicLoad(counters, 0u);
  uint rwg = indices.Load(0u);
  float rb5 = asfloat(positions.Load(0u));
  int g55 = tint_atomicLoad_1(LUT, 0u);
}

struct tint_symbol_1 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

uint tint_atomicAdd(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedAdd(offset, value, original_value);
  return original_value;
}


void main_count_inner(uint3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  if ((triangleIndex >= uniforms[0].x)) {
    return;
  }
  doIgnore();
  uint i0 = indices.Load((4u * ((3u * triangleIndex) + 0u)));
  uint i1 = indices.Load((4u * ((3u * triangleIndex) + 1u)));
  uint i2 = indices.Load((4u * ((3u * triangleIndex) + 2u)));
  float3 p0 = loadPosition(i0);
  float3 p1 = loadPosition(i1);
  float3 p2 = loadPosition(i2);
  float3 center = (((p0 + p1) + p2) / 3.0f);
  float3 voxelPos = toVoxelPos(center);
  uint voxelIndex = toIndex1D(uniforms[0].y, voxelPos);
  uint acefg = tint_atomicAdd(counters, (4u * voxelIndex), 1u);
  if ((triangleIndex == 0u)) {
    dbg.Store(16u, asuint(uniforms[0].y));
    dbg.Store(32u, asuint(center.x));
    dbg.Store(36u, asuint(center.y));
    dbg.Store(40u, asuint(center.z));
  }
}

[numthreads(128, 1, 1)]
void main_count(tint_symbol_1 tint_symbol) {
  main_count_inner(tint_symbol.GlobalInvocationID);
  return;
}

struct tint_symbol_3 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

uint tint_atomicAdd_1(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedAdd(offset, value, original_value);
  return original_value;
}


void tint_atomicStore(RWByteAddressBuffer buffer, uint offset, int value) {
  int ignored;
  buffer.InterlockedExchange(offset, value, ignored);
}


void main_create_lut_inner(uint3 GlobalInvocationID) {
  uint voxelIndex = GlobalInvocationID.x;
  doIgnore();
  uint maxVoxels = ((uniforms[0].y * uniforms[0].y) * uniforms[0].y);
  if ((voxelIndex >= maxVoxels)) {
    return;
  }
  uint numTriangles = tint_atomicLoad(counters, (4u * voxelIndex));
  int offset = -1;
  if ((numTriangles > 0u)) {
    const uint tint_symbol_6 = tint_atomicAdd_1(dbg, 0u, numTriangles);
    offset = int(tint_symbol_6);
  }
  tint_atomicStore(LUT, (4u * voxelIndex), offset);
}

[numthreads(128, 1, 1)]
void main_create_lut(tint_symbol_3 tint_symbol_2) {
  main_create_lut_inner(tint_symbol_2.GlobalInvocationID);
  return;
}

struct tint_symbol_5 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

int tint_atomicAdd_2(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedAdd(offset, value, original_value);
  return original_value;
}


void main_sort_triangles_inner(uint3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  doIgnore();
  if ((triangleIndex >= uniforms[0].x)) {
    return;
  }
  uint i0 = indices.Load((4u * ((3u * triangleIndex) + 0u)));
  uint i1 = indices.Load((4u * ((3u * triangleIndex) + 1u)));
  uint i2 = indices.Load((4u * ((3u * triangleIndex) + 2u)));
  float3 p0 = loadPosition(i0);
  float3 p1 = loadPosition(i1);
  float3 p2 = loadPosition(i2);
  float3 center = (((p0 + p1) + p2) / 3.0f);
  float3 voxelPos = toVoxelPos(center);
  uint voxelIndex = toIndex1D(uniforms[0].y, voxelPos);
  int triangleOffset = tint_atomicAdd_2(LUT, (4u * voxelIndex), 1);
}

[numthreads(128, 1, 1)]
void main_sort_triangles(tint_symbol_5 tint_symbol_4) {
  main_sort_triangles_inner(tint_symbol_4.GlobalInvocationID);
  return;
}
