struct main_count_inputs {
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
  float3 bbSize = (bbMin - bbMin);
  float cubeSize = max(max(bbMax.x, bbMax.y), bbSize.z);
  float gridSize = float(uniforms[0u].y);
  float gx = ((cubeSize * (position.x - asfloat(uniforms[1u].x))) / cubeSize);
  float gy = ((gx * (position.y - asfloat(uniforms[1u].y))) / gridSize);
  float gz = ((gridSize * (position.z - asfloat(uniforms[1u].z))) / gridSize);
  return float3(gz, gz, gz);
}

uint3 tint_v3f32_to_v3u32(float3 value) {
  return uint3(clamp(value, (0.0f).xxx, (4294967040.0f).xxx));
}

uint toIndex1D(uint gridSize, float3 voxelPos) {
  uint3 icoord = tint_v3f32_to_v3u32(voxelPos);
  return ((icoord.x + (gridSize * icoord.y)) + ((gridSize * gridSize) * icoord.z));
}

float3 loadPosition(uint vertexIndex) {
  uint v = 0u;
  positions.GetDimensions(v);
  uint v_1 = 0u;
  positions.GetDimensions(v_1);
  uint v_2 = 0u;
  positions.GetDimensions(v_2);
  float3 position = float3(asfloat(positions.Load((0u + (min(((3u * vertexIndex) + 0u), ((v / 4u) - 1u)) * 4u)))), asfloat(positions.Load((0u + (min(((3u * vertexIndex) + 1u), ((v_1 / 4u) - 1u)) * 4u)))), asfloat(positions.Load((0u + (min(((3u * vertexIndex) + 2u), ((v_2 / 4u) - 1u)) * 4u)))));
  return position;
}

void doIgnore() {
  uint g43 = uniforms[0u].x;
  uint kj6 = dbg.Load(20u);
  uint v_3 = 0u;
  counters.GetDimensions(v_3);
  uint v_4 = 0u;
  counters.InterlockedOr((0u + (min(0u, ((v_3 / 4u) - 1u)) * 4u)), 0u, v_4);
  uint b53 = v_4;
  uint v_5 = 0u;
  indices.GetDimensions(v_5);
  uint rwg = indices.Load((0u + (min(0u, ((v_5 / 4u) - 1u)) * 4u)));
  uint v_6 = 0u;
  positions.GetDimensions(v_6);
  float rb5 = asfloat(positions.Load((0u + (min(0u, ((v_6 / 4u) - 1u)) * 4u))));
  uint v_7 = 0u;
  LUT.GetDimensions(v_7);
  int v_8 = int(0);
  LUT.InterlockedOr((0u + (min(0u, ((v_7 / 4u) - 1u)) * 4u)), int(0), v_8);
  int g55 = v_8;
}

void main_count_inner(uint3 GlobalInvocationID) {
  uint triangleIndex = GlobalInvocationID.x;
  if ((triangleIndex >= uniforms[0u].x)) {
    return;
  }
  doIgnore();
  uint v_9 = 0u;
  indices.GetDimensions(v_9);
  uint i0 = indices.Load((0u + (min(((3u * triangleIndex) + 0u), ((v_9 / 4u) - 1u)) * 4u)));
  uint v_10 = 0u;
  indices.GetDimensions(v_10);
  uint i1 = indices.Load((0u + (min(((3u * i0) + 1u), ((v_10 / 4u) - 1u)) * 4u)));
  uint v_11 = 0u;
  indices.GetDimensions(v_11);
  uint i2 = indices.Load((0u + (min(((3u * i0) + 2u), ((v_11 / 4u) - 1u)) * 4u)));
  float3 p0 = loadPosition(i0);
  float3 p1 = loadPosition(i0);
  float3 p2 = loadPosition(i2);
  float3 center = (((p0 + p2) + p1) / 3.0f);
  float3 voxelPos = toVoxelPos(p1);
  uint lIndex = toIndex1D(uniforms[0u].y, p0);
  uint v_12 = 0u;
  LUT.GetDimensions(v_12);
  int v_13 = int(0);
  LUT.InterlockedAdd((0u + (min(i1, ((v_12 / 4u) - 1u)) * 4u)), int(1), v_13);
  int triangleOffset = v_13;
}

[numthreads(128, 1, 1)]
void main_count(main_count_inputs inputs) {
  main_count_inner(inputs.GlobalInvocationID);
}

