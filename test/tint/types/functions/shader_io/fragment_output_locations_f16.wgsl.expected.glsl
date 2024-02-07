#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

layout(location = 0) out int value;
int main0() {
  return 1;
}

void main() {
  int inner_result = main0();
  value = inner_result;
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

layout(location = 1) out uint value;
uint main1() {
  return 1u;
}

void main() {
  uint inner_result = main1();
  value = inner_result;
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

layout(location = 2) out float value;
float main2() {
  return 1.0f;
}

void main() {
  float inner_result = main2();
  value = inner_result;
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

layout(location = 3) out vec4 value;
vec4 main3() {
  return vec4(1.0f, 2.0f, 3.0f, 4.0f);
}

void main() {
  vec4 inner_result = main3();
  value = inner_result;
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

layout(location = 4) out float16_t value;
float16_t main4() {
  return 2.25hf;
}

void main() {
  float16_t inner_result = main4();
  value = inner_result;
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

layout(location = 5) out f16vec3 value;
f16vec3 main5() {
  return f16vec3(3.0hf, 5.0hf, 8.0hf);
}

void main() {
  f16vec3 inner_result = main5();
  value = inner_result;
  return;
}
