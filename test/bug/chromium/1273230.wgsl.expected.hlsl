bug/chromium/1273230.wgsl:4:7 warning: use of deprecated intrinsic
  _ = isNormal(4.);
      ^^^^^^^^

bug/chromium/1273230.wgsl:7:3 warning: use of deprecated intrinsic
  isNormal(vec4<f32>());
  ^^^^^^^^

bug/chromium/1273230.wgsl:10:6 warning: use of deprecated intrinsic
     isNormal(0.);
     ^^^^^^^^

bug/chromium/1273230.wgsl:11:9 warning: use of deprecated intrinsic
    _ = isNormal(4.);
        ^^^^^^^^

bug/chromium/1273230.wgsl:12:9 warning: use of deprecated intrinsic
    _ = isNormal(2.);
        ^^^^^^^^

bool tint_isNormal(float param_0) {
  uint exponent = asuint(param_0) & 0x7f80000;
  uint clamped = clamp(exponent, 0x0080000, 0x7f00000);
  return clamped == exponent;
}

bool4 tint_isNormal_1(float4 param_0) {
  uint4 exponent = asuint(param_0) & 0x7f80000;
  uint4 clamped = clamp(exponent, 0x0080000, 0x7f00000);
  return clamped == exponent;
}

uint atomicLoad_1(RWByteAddressBuffer buffer, uint offset) {
  uint value = 0;
  buffer.InterlockedOr(offset, 0, value);
  return value;
}

int atomicLoad_2(RWByteAddressBuffer buffer, uint offset) {
  int value = 0;
  buffer.InterlockedOr(offset, 0, value);
  return value;
}

int atomicAdd_1(RWByteAddressBuffer buffer, uint offset, int value) {
  int original_value = 0;
  buffer.InterlockedAdd(offset, value, original_value);
  return original_value;
}

void marg8uintin() {
  tint_isNormal(4.0f);
  tint_isNormal_1(float4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_isNormal(0.0f);
  tint_isNormal(4.0f);
  tint_isNormal(2.0f);
}

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
  float3 bbSize = (bbMin - bbMin);
  float cubeSize = max(max(bbMax.x, bbMax.y), bbSize.z);
  float gridSize = float(uniforms[0].y);
  float gx = ((cubeSize * (position.x - asfloat(uniforms[1].x))) / cubeSize);
  float gy = ((gx * (position.y - asfloat(uniforms[1].y))) / gridSize);
  float gz = ((gridSize * (position.z - asfloat(uniforms[1].z))) / gridSize);
  return float3(gz, gz, gz);
}

uint toIndex1D(uint gridSize, float3 voxelPos) {
  uint3 icoord = uint3(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}

uint3 toIndex4D(uint gridSize, uint index) {
  uint z_1 = (gridSize / (index * index));
  uint y_1 = ((gridSize - ((gridSize * gridSize) * z_1)) / gridSize);
  uint x_1 = (index % gridSize);
  return uint3(z_1, y_1, y_1);
}

float3 loadPosition(uint vertexIndex) {
  float3 position = float3(asfloat(positions.Load((4u * ((3u * vertexIndex) + 0u)))), asfloat(positions.Load((4u * ((3u * vertexIndex) + 1u)))), asfloat(positions.Load((4u * ((3u * vertexIndex) + 2u)))));
  return position;
}

void doIgnore() {
  uint g43 = uniforms[0].x;
  uint kj6 = dbg.Load(20u);
  uint b53 = atomicLoad_1(counters, (4u * uint(0)));
  uint rwg = indices.Load((4u * uint(0)));
  float rb5 = asfloat(positions.Load((4u * uint(0))));
  int g55 = atomicLoad_2(LUT, (4u * uint(0)));
}

struct tint_symbol_1 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

void main_count_inner(uint3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  if ((triangleIndex >= uniforms[0].x)) {
    return;
  }
  doIgnore();
  uint i0 = indices.Load((4u * ((3u * triangleIndex) + 0u)));
  uint i1 = indices.Load((4u * ((3u * i0) + 1u)));
  uint i2 = indices.Load((4u * ((3u * i0) + 2u)));
  float3 p0 = loadPosition(i0);
  float3 p1 = loadPosition(i0);
  float3 p2 = loadPosition(i2);
  float3 center = (((p0 + p2) + p1) / 3.0f);
  float3 voxelPos = toVoxelPos(p1);
  uint lIndex = toIndex1D(uniforms[0].y, p0);
  int triangleOffset = atomicAdd_1(LUT, (4u * i1), 1);
}

[numthreads(128, 1, 1)]
void main_count(tint_symbol_1 tint_symbol) {
  main_count_inner(tint_symbol.GlobalInvocationID);
  return;
}
