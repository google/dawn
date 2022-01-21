uint atomicAdd_1(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedAdd(offset, value, original_value);
  return original_value;
}

ByteAddressBuffer tables : register(t0, space0);

RWByteAddressBuffer volume : register(u1, space0);

RWByteAddressBuffer positionsOut : register(u2, space0);

RWByteAddressBuffer normalsOut : register(u3, space0);

RWByteAddressBuffer indicesOut : register(u4, space0);

RWByteAddressBuffer drawOut : register(u5, space0);

float valueAt(uint3 index) {
  if (any((index >= volume.Load3(48u)))) {
    return 0.0f;
  }
  const uint valueIndex = ((index.x + (index.y * volume.Load(48u))) + ((index.z * volume.Load(48u)) * volume.Load(52u)));
  return asfloat(volume.Load((64u + (4u * valueIndex))));
}

float3 positionAt(uint3 index) {
  return (asfloat(volume.Load3(0u)) + (asfloat(volume.Load3(32u)) * float3(index.xyz)));
}

float3 normalAt(uint3 index) {
  return float3((valueAt((index - uint3(1u, 0u, 0u))) - valueAt((index + uint3(1u, 0u, 0u)))), (valueAt((index - uint3(0u, 1u, 0u))) - valueAt((index + uint3(0u, 1u, 0u)))), (valueAt((index - uint3(0u, 0u, 1u))) - valueAt((index + uint3(0u, 0u, 1u)))));
}

static float3 positions[12] = (float3[12])0;
static float3 normals[12] = (float3[12])0;
static uint indices[12] = (uint[12])0;
static uint cubeVerts = 0u;

void interpX(uint index, uint3 i, float va, float vb) {
  const float mu = ((asfloat(volume.Load(60u)) - va) / (vb - va));
  positions[cubeVerts] = (positionAt(i) + float3((asfloat(volume.Load(32u)) * mu), 0.0f, 0.0f));
  const float3 na = normalAt(i);
  const float3 nb = normalAt((i + uint3(1u, 0u, 0u)));
  normals[cubeVerts] = lerp(na, nb, float3(mu, mu, mu));
  indices[index] = cubeVerts;
  cubeVerts = (cubeVerts + 1u);
}

void interpY(uint index, uint3 i, float va, float vb) {
  const float mu = ((asfloat(volume.Load(60u)) - va) / (vb - va));
  positions[cubeVerts] = (positionAt(i) + float3(0.0f, (asfloat(volume.Load(36u)) * mu), 0.0f));
  const float3 na = normalAt(i);
  const float3 nb = normalAt((i + uint3(0u, 1u, 0u)));
  normals[cubeVerts] = lerp(na, nb, float3(mu, mu, mu));
  indices[index] = cubeVerts;
  cubeVerts = (cubeVerts + 1u);
}

void interpZ(uint index, uint3 i, float va, float vb) {
  const float mu = ((asfloat(volume.Load(60u)) - va) / (vb - va));
  positions[cubeVerts] = (positionAt(i) + float3(0.0f, 0.0f, (asfloat(volume.Load(40u)) * mu)));
  const float3 na = normalAt(i);
  const float3 nb = normalAt((i + uint3(0u, 0u, 1u)));
  normals[cubeVerts] = lerp(na, nb, float3(mu, mu, mu));
  indices[index] = cubeVerts;
  cubeVerts = (cubeVerts + 1u);
}

struct tint_symbol_1 {
  uint3 global_id : SV_DispatchThreadID;
};

void computeMain_inner(uint3 global_id) {
  const uint3 i0 = global_id;
  const uint3 i1 = (global_id + uint3(1u, 0u, 0u));
  const uint3 i2 = (global_id + uint3(1u, 1u, 0u));
  const uint3 i3 = (global_id + uint3(0u, 1u, 0u));
  const uint3 i4 = (global_id + uint3(0u, 0u, 1u));
  const uint3 i5 = (global_id + uint3(1u, 0u, 1u));
  const uint3 i6 = (global_id + uint3(1u, 1u, 1u));
  const uint3 i7 = (global_id + uint3(0u, 1u, 1u));
  const float v0 = valueAt(i0);
  const float v1 = valueAt(i1);
  const float v2 = valueAt(i2);
  const float v3 = valueAt(i3);
  const float v4 = valueAt(i4);
  const float v5 = valueAt(i5);
  const float v6 = valueAt(i6);
  const float v7 = valueAt(i7);
  uint cubeIndex = 0u;
  if ((v0 < asfloat(volume.Load(60u)))) {
    cubeIndex = (cubeIndex | 1u);
  }
  if ((v1 < asfloat(volume.Load(60u)))) {
    cubeIndex = (cubeIndex | 2u);
  }
  if ((v2 < asfloat(volume.Load(60u)))) {
    cubeIndex = (cubeIndex | 4u);
  }
  if ((v3 < asfloat(volume.Load(60u)))) {
    cubeIndex = (cubeIndex | 8u);
  }
  if ((v4 < asfloat(volume.Load(60u)))) {
    cubeIndex = (cubeIndex | 16u);
  }
  if ((v5 < asfloat(volume.Load(60u)))) {
    cubeIndex = (cubeIndex | 32u);
  }
  if ((v6 < asfloat(volume.Load(60u)))) {
    cubeIndex = (cubeIndex | 64u);
  }
  if ((v7 < asfloat(volume.Load(60u)))) {
    cubeIndex = (cubeIndex | 128u);
  }
  const uint edges = tables.Load((4u * cubeIndex));
  if (((edges & 1u) != 0u)) {
    interpX(0u, i0, v0, v1);
  }
  if (((edges & 2u) != 0u)) {
    interpY(1u, i1, v1, v2);
  }
  if (((edges & 4u) != 0u)) {
    interpX(2u, i3, v3, v2);
  }
  if (((edges & 8u) != 0u)) {
    interpY(3u, i0, v0, v3);
  }
  if (((edges & 16u) != 0u)) {
    interpX(4u, i4, v4, v5);
  }
  if (((edges & 32u) != 0u)) {
    interpY(5u, i5, v5, v6);
  }
  if (((edges & 64u) != 0u)) {
    interpX(6u, i7, v7, v6);
  }
  if (((edges & 128u) != 0u)) {
    interpY(7u, i4, v4, v7);
  }
  if (((edges & 256u) != 0u)) {
    interpZ(8u, i0, v0, v4);
  }
  if (((edges & 512u) != 0u)) {
    interpZ(9u, i1, v1, v5);
  }
  if (((edges & 1024u) != 0u)) {
    interpZ(10u, i2, v2, v6);
  }
  if (((edges & 2048u) != 0u)) {
    interpZ(11u, i3, v3, v7);
  }
  const uint triTableOffset = ((cubeIndex << 4u) + 1u);
  const uint indexCount = uint(asint(tables.Load((1024u + (4u * (triTableOffset - 1u))))));
  uint firstVertex = atomicAdd_1(drawOut, 4u, cubeVerts);
  const uint bufferOffset = ((global_id.x + (global_id.y * volume.Load(48u))) + ((global_id.z * volume.Load(48u)) * volume.Load(52u)));
  const uint firstIndex = (bufferOffset * 15u);
  {
    [loop] for(uint i = 0u; (i < cubeVerts); i = (i + 1u)) {
      positionsOut.Store((4u * ((firstVertex * 3u) + (i * 3u))), asuint(positions[i].x));
      positionsOut.Store((4u * (((firstVertex * 3u) + (i * 3u)) + 1u)), asuint(positions[i].y));
      positionsOut.Store((4u * (((firstVertex * 3u) + (i * 3u)) + 2u)), asuint(positions[i].z));
      normalsOut.Store((4u * ((firstVertex * 3u) + (i * 3u))), asuint(normals[i].x));
      normalsOut.Store((4u * (((firstVertex * 3u) + (i * 3u)) + 1u)), asuint(normals[i].y));
      normalsOut.Store((4u * (((firstVertex * 3u) + (i * 3u)) + 2u)), asuint(normals[i].z));
    }
  }
  {
    [loop] for(uint i = 0u; (i < indexCount); i = (i + 1u)) {
      const int index = asint(tables.Load((1024u + (4u * (triTableOffset + i)))));
      indicesOut.Store((4u * (firstIndex + i)), asuint((firstVertex + indices[index])));
    }
  }
  {
    [loop] for(uint i = indexCount; (i < 15u); i = (i + 1u)) {
      indicesOut.Store((4u * (firstIndex + i)), asuint(firstVertex));
    }
  }
}

[numthreads(4, 4, 4)]
void computeMain(tint_symbol_1 tint_symbol) {
  computeMain_inner(tint_symbol.global_id);
  return;
}
