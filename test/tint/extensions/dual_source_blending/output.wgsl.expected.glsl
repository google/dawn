#version 310 es
#extension GL_EXT_blend_func_extended : require
precision highp float;
precision highp int;

layout(location = 0, index = 0) out vec4 color_1;
layout(location = 0, index = 1) out vec4 blend_1;
struct FragOutput {
  vec4 color;
  vec4 blend;
};

FragOutput frag_main() {
  FragOutput tint_symbol = FragOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol.color = vec4(0.5f, 0.5f, 0.5f, 1.0f);
  tint_symbol.blend = vec4(0.5f, 0.5f, 0.5f, 1.0f);
  return tint_symbol;
}

void main() {
  FragOutput inner_result = frag_main();
  color_1 = inner_result.color;
  blend_1 = inner_result.blend;
  return;
}
