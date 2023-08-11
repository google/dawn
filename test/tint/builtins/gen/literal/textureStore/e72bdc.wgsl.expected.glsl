#version 310 es

layout(rgba16f) uniform highp writeonly image2DArray arg_0;
void textureStore_e72bdc() {
  imageStore(arg_0, ivec3(ivec2(1), int(1u)), vec4(1.0f));
}

vec4 vertex_main() {
  textureStore_e72bdc();
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
precision highp float;

layout(rgba16f) uniform highp writeonly image2DArray arg_0;
void textureStore_e72bdc() {
  imageStore(arg_0, ivec3(ivec2(1), int(1u)), vec4(1.0f));
}

void fragment_main() {
  textureStore_e72bdc();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(rgba16f) uniform highp writeonly image2DArray arg_0;
void textureStore_e72bdc() {
  imageStore(arg_0, ivec3(ivec2(1), int(1u)), vec4(1.0f));
}

void compute_main() {
  textureStore_e72bdc();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
