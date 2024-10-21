#version 310 es
#extension GL_EXT_blend_func_extended: require
precision highp float;
precision highp int;


struct FragOutput {
  vec4 color;
  vec4 blend;
};

struct FragInput {
  vec4 a;
  vec4 b;
};

layout(location = 0) in vec4 frag_main_loc0_Input;
layout(location = 1) in vec4 frag_main_loc1_Input;
layout(location = 0, index = 0) out vec4 frag_main_loc0_idx0_Output;
layout(location = 0, index = 1) out vec4 frag_main_loc0_idx1_Output;
FragOutput frag_main_inner(FragInput tint_symbol) {
  FragOutput tint_symbol_1 = FragOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol_1.color = tint_symbol.a;
  tint_symbol_1.blend = tint_symbol.b;
  return tint_symbol_1;
}
void main() {
  FragOutput v = frag_main_inner(FragInput(frag_main_loc0_Input, frag_main_loc1_Input));
  frag_main_loc0_idx0_Output = v.color;
  frag_main_loc0_idx1_Output = v.blend;
}
