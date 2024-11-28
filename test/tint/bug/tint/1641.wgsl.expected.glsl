#version 310 es


struct Normals {
  vec3 f;
};

vec4 tint_symbol_inner() {
  int zero = 0;
  return vec4(Normals[1](Normals(vec3(0.0f, 0.0f, 1.0f)))[min(uint(zero), 0u)].f, 1.0f);
}
void main() {
  gl_Position = tint_symbol_inner();
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
