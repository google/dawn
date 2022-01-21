#version 310 es
precision mediump float;

struct Tables {
  uint edges[256];
  int tris[4096];
};

layout (binding = 0) buffer Tables_1 {
  uint edges[256];
  int tris[4096];
} tables;

layout (binding = 1) buffer IsosurfaceVolume_1 {
  vec3 tint_symbol;
  vec3 tint_symbol_1;
  vec3 tint_symbol_2;
  uvec3 size;
  float threshold;
  float values[];
} volume;

layout (binding = 2) buffer PositionBuffer_1 {
  float values[];
} positionsOut;

layout (binding = 3) buffer NormalBuffer_1 {
  float values[];
} normalsOut;

layout (binding = 4) buffer IndexBuffer_1 {
  uint tris[];
} indicesOut;

struct DrawIndirectArgs {
  uint vc;
  uint vertexCount;
  uint firstVertex;
  uint firstInstance;
  uint indexCount;
  uint indexedInstanceCount;
  uint indexedFirstIndex;
  uint indexedBaseVertex;
  uint indexedFirstInstance;
};

layout (binding = 5) buffer DrawIndirectArgs_1 {
  uint vc;
  uint vertexCount;
  uint firstVertex;
  uint firstInstance;
  uint indexCount;
  uint indexedInstanceCount;
  uint indexedFirstIndex;
  uint indexedBaseVertex;
  uint indexedFirstInstance;
} drawOut;

float valueAt(uvec3 index) {
  if (any(greaterThanEqual(index, volume.size))) {
    return 0.0f;
  }
  uint valueIndex = ((index.x + (index.y * volume.size.x)) + ((index.z * volume.size.x) * volume.size.y));
  return volume.values[valueIndex];
}

vec3 positionAt(uvec3 index) {
  return (volume.tint_symbol + (volume.tint_symbol_2 * vec3(index.xyz)));
}

vec3 normalAt(uvec3 index) {
  return vec3((valueAt((index - uvec3(1u, 0u, 0u))) - valueAt((index + uvec3(1u, 0u, 0u)))), (valueAt((index - uvec3(0u, 1u, 0u))) - valueAt((index + uvec3(0u, 1u, 0u)))), (valueAt((index - uvec3(0u, 0u, 1u))) - valueAt((index + uvec3(0u, 0u, 1u)))));
}

vec3 positions[12] = vec3[12](vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
vec3 normals[12] = vec3[12](vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
uint indices[12] = uint[12](0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
uint cubeVerts = 0u;

void interpX(uint index, uvec3 i, float va, float vb) {
  float mu = ((volume.threshold - va) / (vb - va));
  positions[cubeVerts] = (positionAt(i) + vec3((volume.tint_symbol_2.x * mu), 0.0f, 0.0f));
  vec3 na = normalAt(i);
  vec3 nb = normalAt((i + uvec3(1u, 0u, 0u)));
  normals[cubeVerts] = mix(na, nb, vec3(mu, mu, mu));
  indices[index] = cubeVerts;
  cubeVerts = (cubeVerts + 1u);
}

void interpY(uint index, uvec3 i, float va, float vb) {
  float mu = ((volume.threshold - va) / (vb - va));
  positions[cubeVerts] = (positionAt(i) + vec3(0.0f, (volume.tint_symbol_2.y * mu), 0.0f));
  vec3 na = normalAt(i);
  vec3 nb = normalAt((i + uvec3(0u, 1u, 0u)));
  normals[cubeVerts] = mix(na, nb, vec3(mu, mu, mu));
  indices[index] = cubeVerts;
  cubeVerts = (cubeVerts + 1u);
}

void interpZ(uint index, uvec3 i, float va, float vb) {
  float mu = ((volume.threshold - va) / (vb - va));
  positions[cubeVerts] = (positionAt(i) + vec3(0.0f, 0.0f, (volume.tint_symbol_2.z * mu)));
  vec3 na = normalAt(i);
  vec3 nb = normalAt((i + uvec3(0u, 0u, 1u)));
  normals[cubeVerts] = mix(na, nb, vec3(mu, mu, mu));
  indices[index] = cubeVerts;
  cubeVerts = (cubeVerts + 1u);
}

struct tint_symbol_4 {
  uvec3 global_id;
};

void computeMain_inner(uvec3 global_id) {
  uvec3 i0 = global_id;
  uvec3 i1 = (global_id + uvec3(1u, 0u, 0u));
  uvec3 i2 = (global_id + uvec3(1u, 1u, 0u));
  uvec3 i3 = (global_id + uvec3(0u, 1u, 0u));
  uvec3 i4 = (global_id + uvec3(0u, 0u, 1u));
  uvec3 i5 = (global_id + uvec3(1u, 0u, 1u));
  uvec3 i6 = (global_id + uvec3(1u, 1u, 1u));
  uvec3 i7 = (global_id + uvec3(0u, 1u, 1u));
  float v0 = valueAt(i0);
  float v1 = valueAt(i1);
  float v2 = valueAt(i2);
  float v3 = valueAt(i3);
  float v4 = valueAt(i4);
  float v5 = valueAt(i5);
  float v6 = valueAt(i6);
  float v7 = valueAt(i7);
  uint cubeIndex = 0u;
  if ((v0 < volume.threshold)) {
    cubeIndex = (cubeIndex | 1u);
  }
  if ((v1 < volume.threshold)) {
    cubeIndex = (cubeIndex | 2u);
  }
  if ((v2 < volume.threshold)) {
    cubeIndex = (cubeIndex | 4u);
  }
  if ((v3 < volume.threshold)) {
    cubeIndex = (cubeIndex | 8u);
  }
  if ((v4 < volume.threshold)) {
    cubeIndex = (cubeIndex | 16u);
  }
  if ((v5 < volume.threshold)) {
    cubeIndex = (cubeIndex | 32u);
  }
  if ((v6 < volume.threshold)) {
    cubeIndex = (cubeIndex | 64u);
  }
  if ((v7 < volume.threshold)) {
    cubeIndex = (cubeIndex | 128u);
  }
  uint edges = tables.edges[cubeIndex];
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
  uint triTableOffset = ((cubeIndex << 4u) + 1u);
  uint indexCount = uint(tables.tris[(triTableOffset - 1u)]);
  uint firstVertex = atomicAdd(drawOut.vertexCount, cubeVerts);
  uint bufferOffset = ((global_id.x + (global_id.y * volume.size.x)) + ((global_id.z * volume.size.x) * volume.size.y));
  uint firstIndex = (bufferOffset * 15u);
  {
    for(uint i = 0u; (i < cubeVerts); i = (i + 1u)) {
      positionsOut.values[((firstVertex * 3u) + (i * 3u))] = positions[i].x;
      positionsOut.values[(((firstVertex * 3u) + (i * 3u)) + 1u)] = positions[i].y;
      positionsOut.values[(((firstVertex * 3u) + (i * 3u)) + 2u)] = positions[i].z;
      normalsOut.values[((firstVertex * 3u) + (i * 3u))] = normals[i].x;
      normalsOut.values[(((firstVertex * 3u) + (i * 3u)) + 1u)] = normals[i].y;
      normalsOut.values[(((firstVertex * 3u) + (i * 3u)) + 2u)] = normals[i].z;
    }
  }
  {
    for(uint i = 0u; (i < indexCount); i = (i + 1u)) {
      int index = tables.tris[(triTableOffset + i)];
      indicesOut.tris[(firstIndex + i)] = (firstVertex + indices[index]);
    }
  }
  {
    for(uint i = indexCount; (i < 15u); i = (i + 1u)) {
      indicesOut.tris[(firstIndex + i)] = firstVertex;
    }
  }
}

layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
void computeMain(tint_symbol_4 tint_symbol_3) {
  computeMain_inner(tint_symbol_3.global_id);
  return;
}
void main() {
  tint_symbol_4 inputs;
  inputs.global_id = gl_GlobalInvocationID;
  computeMain(inputs);
}


