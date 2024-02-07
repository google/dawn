#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(location = 1) out float col1_1;
layout(location = 2) out float16_t col2_1;
struct Interface {
  float col1;
  float16_t col2;
  vec4 pos;
};

Interface vert_main() {
  Interface tint_symbol = Interface(0.40000000596046447754f, 0.599609375hf, vec4(0.0f));
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  Interface inner_result = vert_main();
  col1_1 = inner_result.col1;
  col2_1 = inner_result.col2;
  gl_Position = inner_result.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

layout(location = 1) in float col1_1;
layout(location = 2) in float16_t col2_1;
struct Interface {
  float col1;
  float16_t col2;
  vec4 pos;
};

void frag_main(Interface colors) {
  float r = colors.col1;
  float16_t g = colors.col2;
}

void main() {
  Interface tint_symbol = Interface(col1_1, col2_1, gl_FragCoord);
  frag_main(tint_symbol);
  return;
}
