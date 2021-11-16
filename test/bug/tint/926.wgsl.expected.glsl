SKIP: FAILED

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
  uint atomic_result = 0u;
  InterlockedAdd(drawOut.vertexCount, cubeVerts, atomic_result);
  uint firstVertex = atomic_result;
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


Error parsing GLSL shader:
ERROR: 0:16: 'InterlockedAdd' : no matching overloaded function found 
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



