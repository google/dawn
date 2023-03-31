uint3 tint_ftou(float3 v) {
  return ((v < (4294967040.0f).xxx) ? ((v < (0.0f).xxx) ? (0u).xxx : uint3(v)) : (4294967295u).xxx);
}

cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[3];
};
RWByteAddressBuffer indices : register(u10);
RWByteAddressBuffer positions : register(u11);
RWByteAddressBuffer counters : register(u20);
RWByteAddressBuffer LUT : register(u21);
RWByteAddressBuffer dbg : register(u50);

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
  uint3 icoord = tint_ftou(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}

uint tint_div(uint lhs, uint rhs) {
  return (lhs / ((rhs == 0u) ? 1u : rhs));
}

uint tint_mod(uint lhs, uint rhs) {
  return (lhs % ((rhs == 0u) ? 1u : rhs));
}

uint3 toIndex3D(uint gridSize, uint index) {
  uint z = tint_div(index, (gridSize * gridSize));
  uint y = tint_div((index - ((gridSize * gridSize) * z)), gridSize);
  uint x = tint_mod(index, gridSize);
  return uint3(x, y, z);
}

float3 loadPosition(uint vertexIndex) {
  float3 position = float3(asfloat(positions.Load((4u * ((3u * vertexIndex) + 0u)))), asfloat(positions.Load((4u * ((3u * vertexIndex) + 1u)))), asfloat(positions.Load((4u * ((3u * vertexIndex) + 2u)))));
  return position;
}

uint countersatomicLoad(uint offset) {
  uint value = 0;
  counters.InterlockedOr(offset, 0, value);
  return value;
}


int LUTatomicLoad(uint offset) {
  int value = 0;
  LUT.InterlockedOr(offset, 0, value);
  return value;
}


void doIgnore() {
  uint g42 = uniforms[0].x;
  uint kj6 = dbg.Load(20u);
  uint b53 = countersatomicLoad(0u);
  uint rwg = indices.Load(0u);
  float rb5 = asfloat(positions.Load(0u));
  int g55 = LUTatomicLoad(0u);
}

struct tint_symbol_2 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

uint countersatomicAdd(uint offset, uint value) {
  uint original_value = 0;
  counters.InterlockedAdd(offset, value, original_value);
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
  uint acefg = countersatomicAdd((4u * voxelIndex), 1u);
  if ((triangleIndex == 0u)) {
    dbg.Store(16u, asuint(uniforms[0].y));
    dbg.Store(32u, asuint(center.x));
    dbg.Store(36u, asuint(center.y));
    dbg.Store(40u, asuint(center.z));
  }
}

[numthreads(128, 1, 1)]
void main_count(tint_symbol_2 tint_symbol_1) {
  main_count_inner(tint_symbol_1.GlobalInvocationID);
  return;
}

struct tint_symbol_4 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

uint dbgatomicAdd(uint offset, uint value) {
  uint original_value = 0;
  dbg.InterlockedAdd(offset, value, original_value);
  return original_value;
}


void LUTatomicStore(uint offset, int value) {
  int ignored;
  LUT.InterlockedExchange(offset, value, ignored);
}


void main_create_lut_inner(uint3 GlobalInvocationID) {
  uint voxelIndex = GlobalInvocationID.x;
  doIgnore();
  uint maxVoxels = ((uniforms[0].y * uniforms[0].y) * uniforms[0].y);
  if ((voxelIndex >= maxVoxels)) {
    return;
  }
  uint numTriangles = countersatomicLoad((4u * voxelIndex));
  int offset = -1;
  if ((numTriangles > 0u)) {
    const uint tint_symbol = dbgatomicAdd(0u, numTriangles);
    offset = int(tint_symbol);
  }
  LUTatomicStore((4u * voxelIndex), offset);
}

[numthreads(128, 1, 1)]
void main_create_lut(tint_symbol_4 tint_symbol_3) {
  main_create_lut_inner(tint_symbol_3.GlobalInvocationID);
  return;
}

struct tint_symbol_6 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

int LUTatomicAdd(uint offset, int value) {
  int original_value = 0;
  LUT.InterlockedAdd(offset, value, original_value);
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
  int triangleOffset = LUTatomicAdd((4u * voxelIndex), 1);
}

[numthreads(128, 1, 1)]
void main_sort_triangles(tint_symbol_6 tint_symbol_5) {
  main_sort_triangles_inner(tint_symbol_5.GlobalInvocationID);
  return;
}
