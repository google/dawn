#version 310 es
precision highp float;
precision highp int;


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uniform highp usamplerCube arg_0;
layout(binding = 0, std140)
uniform tint_symbol_3_1_ubo {
  TintTextureUniformData tint_symbol_2;
} v_1;
uint textureNumLevels_c386c8() {
  uint res = v_1.tint_symbol_2.tint_builtin_value_0;
  return res;
}
void main() {
  v.tint_symbol = textureNumLevels_c386c8();
}
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uniform highp usamplerCube arg_0;
layout(binding = 0, std140)
uniform tint_symbol_3_1_ubo {
  TintTextureUniformData tint_symbol_2;
} v_1;
uint textureNumLevels_c386c8() {
  uint res = v_1.tint_symbol_2.tint_builtin_value_0;
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureNumLevels_c386c8();
}
#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

uniform highp usamplerCube arg_0;
layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  TintTextureUniformData tint_symbol_1;
} v;
layout(location = 0) flat out uint vertex_main_loc0_Output;
uint textureNumLevels_c386c8() {
  uint res = v.tint_symbol_1.tint_builtin_value_0;
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureNumLevels_c386c8();
  return tint_symbol;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = v_1.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
