#version 310 es
precision mediump float;


const uint width = 128u;
uniform highp sampler2D tex;
layout (binding = 1) buffer Result_1 {
  float values[];
} result;

struct tint_symbol_2 {
  uvec3 GlobalInvocationId;
};

void tint_symbol_inner(uvec3 GlobalInvocationId) {
  result.values[((GlobalInvocationId.y * width) + GlobalInvocationId.x)] = texelFetch(tex, ivec2(int(GlobalInvocationId.x), int(GlobalInvocationId.y)), 0).x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol(tint_symbol_2 tint_symbol_1) {
  tint_symbol_inner(tint_symbol_1.GlobalInvocationId);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.GlobalInvocationId = gl_GlobalInvocationID;
  tint_symbol(inputs);
}


