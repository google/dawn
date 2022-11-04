#version 310 es

uniform highp sampler2DShadow arg_0_arg_1;

void textureSampleLevel_73e892() {
  float res = textureLod(arg_0_arg_1, vec3(vec2(1.0f), 0.0f), float(1u));
}

vec4 vertex_main() {
  textureSampleLevel_73e892();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

uniform highp sampler2DShadow arg_0_arg_1;

void textureSampleLevel_73e892() {
  float res = textureLod(arg_0_arg_1, vec3(vec2(1.0f), 0.0f), float(1u));
}

void fragment_main() {
  textureSampleLevel_73e892();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uniform highp sampler2DShadow arg_0_arg_1;

void textureSampleLevel_73e892() {
  float res = textureLod(arg_0_arg_1, vec3(vec2(1.0f), 0.0f), float(1u));
}

void compute_main() {
  textureSampleLevel_73e892();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
