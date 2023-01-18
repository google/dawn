#version 310 es

layout(rgba8) uniform highp writeonly image2D tex;
vec4 vertex_main() {
  vec4 value = vec4(1.0f, 2.0f, 3.0f, 4.0f);
  imageStore(tex, ivec2(9, 8), value.bgra);
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
