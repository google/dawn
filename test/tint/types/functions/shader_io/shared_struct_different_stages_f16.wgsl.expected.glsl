//
// vert_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct Interface {
  float col1;
  float16_t col2;
  vec4 pos;
};

layout(location = 1) out float tint_interstage_location1;
layout(location = 2) out float16_t tint_interstage_location2;
Interface vert_main_inner() {
  return Interface(0.40000000596046447754f, 0.599609375hf, vec4(0.0f));
}
void main() {
  Interface v = vert_main_inner();
  tint_interstage_location1 = v.col1;
  tint_interstage_location2 = v.col2;
  gl_Position = vec4(v.pos.x, -(v.pos.y), ((2.0f * v.pos.z) - v.pos.w), v.pos.w);
  gl_PointSize = 1.0f;
}
//
// frag_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct Interface {
  float col1;
  float16_t col2;
  vec4 pos;
};

layout(location = 1) in float tint_interstage_location1;
layout(location = 2) in float16_t tint_interstage_location2;
void frag_main_inner(Interface colors) {
  float r = colors.col1;
  float16_t g = colors.col2;
}
void main() {
  frag_main_inner(Interface(tint_interstage_location1, tint_interstage_location2, gl_FragCoord));
}
