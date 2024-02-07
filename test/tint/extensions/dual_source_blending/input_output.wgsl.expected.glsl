#version 310 es
#extension GL_EXT_blend_func_extended : require
precision highp float;
precision highp int;

layout(location = 0) in vec4 a_1;
layout(location = 1) in vec4 b_1;
layout(location = 0, index = 0) out vec4 color_1;
layout(location = 0, index = 1) out vec4 blend_1;
struct FragInput {
  vec4 a;
  vec4 b;
};

struct FragOutput {
  vec4 color;
  vec4 blend;
};

FragOutput frag_main(FragInput tint_symbol) {
  FragOutput tint_symbol_1 = FragOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol_1.color = tint_symbol.a;
  tint_symbol_1.blend = tint_symbol.b;
  return tint_symbol_1;
}

void main() {
  FragInput tint_symbol_2 = FragInput(a_1, b_1);
  FragOutput inner_result = frag_main(tint_symbol_2);
  color_1 = inner_result.color;
  blend_1 = inner_result.blend;
  return;
}
