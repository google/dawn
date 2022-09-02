#version 310 es

layout(binding = 5, std430) buffer DrawIndirectArgs_ssbo {
  uint vertexCount;
} drawOut;

uint cubeVerts = 0u;
void computeMain(uvec3 global_id) {
  uint firstVertex = atomicAdd(drawOut.vertexCount, cubeVerts);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  computeMain(gl_GlobalInvocationID);
  return;
}
