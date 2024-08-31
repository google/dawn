#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct S {
  f16mat3 matrix;
  f16vec3 vector;
};
precision highp float;
precision highp int;


uniform S data;
void main() {
  f16vec3 x = (data.matrix * data.vector);
}
