#version 310 es
precision mediump float;


uniform highp sampler2DArray myTexture;

layout (binding = 3) buffer Result_1 {
  float values[];
} result;

struct tint_symbol_2 {
  uvec3 GlobalInvocationID;
};

void tint_symbol_inner(uvec3 GlobalInvocationID) {
  uint flatIndex = ((((2u * 2u) * GlobalInvocationID.z) + (2u * GlobalInvocationID.y)) + GlobalInvocationID.x);
  flatIndex = (flatIndex * 1u);
  vec4 texel = texelFetch(myTexture, ivec3(ivec2(GlobalInvocationID.xy), 0), 0);
  {
    for(uint i = 0u; (i < 1u); i = (i + 1u)) {
      result.values[(flatIndex + i)] = texel.r;
    }
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol(tint_symbol_2 tint_symbol_1) {
  tint_symbol_inner(tint_symbol_1.GlobalInvocationID);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.GlobalInvocationID = gl_GlobalInvocationID;
  tint_symbol(inputs);
}


