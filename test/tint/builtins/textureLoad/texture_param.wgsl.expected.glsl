#version 310 es

uniform highp isampler2D arg_0;
ivec4 textureLoad2d(ivec2 coords, int level) {
  ivec2 v = ivec2(coords);
  return texelFetch(arg_0, v, int(level));
}
void doTextureLoad() {
  ivec4 res = textureLoad2d(ivec2(0), 0);
}
vec4 vertex_main_inner() {
  doTextureLoad();
  return vec4(0.0f);
}
void main() {
  gl_Position = vertex_main_inner();
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
#version 310 es
precision highp float;
precision highp int;

uniform highp isampler2D arg_0;
ivec4 textureLoad2d(ivec2 coords, int level) {
  ivec2 v = ivec2(coords);
  return texelFetch(arg_0, v, int(level));
}
void doTextureLoad() {
  ivec4 res = textureLoad2d(ivec2(0), 0);
}
void main() {
  doTextureLoad();
}
#version 310 es

uniform highp isampler2D arg_0;
ivec4 textureLoad2d(ivec2 coords, int level) {
  ivec2 v = ivec2(coords);
  return texelFetch(arg_0, v, int(level));
}
void doTextureLoad() {
  ivec4 res = textureLoad2d(ivec2(0), 0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  doTextureLoad();
}
