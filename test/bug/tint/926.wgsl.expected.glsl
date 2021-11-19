#version 310 es
precision mediump float;


layout (binding = 5) buffer DrawIndirectArgs_1 {
  uint vertexCount;
} drawOut;
uint cubeVerts = 0u;

struct tint_symbol_1 {
  uvec3 global_id;
};

void computeMain_inner(uvec3 global_id) {
  uint firstVertex = atomicAdd(drawOut.vertexCount, cubeVerts);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void computeMain(tint_symbol_1 tint_symbol) {
  computeMain_inner(tint_symbol.global_id);
  return;
}
void main() {
  tint_symbol_1 inputs;
  inputs.global_id = gl_GlobalInvocationID;
  computeMain(inputs);
}


