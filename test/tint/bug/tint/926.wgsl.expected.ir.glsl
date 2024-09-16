#version 310 es


struct DrawIndirectArgs {
  uint vertexCount;
};

layout(binding = 5, std430)
buffer tint_symbol_1_1_ssbo {
  DrawIndirectArgs tint_symbol;
} v;
uint cubeVerts = 0u;
void computeMain_inner(uvec3 global_id) {
  uint firstVertex = atomicAdd(v.tint_symbol.vertexCount, cubeVerts);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  computeMain_inner(gl_GlobalInvocationID);
}
