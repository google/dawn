#version 310 es

struct Normals {
  vec3 f;
};

vec4 tint_symbol() {
  int zero = 0;
  Normals tint_symbol_1[1] = Normals[1](Normals(vec3(0.0f, 0.0f, 1.0f)));
  return vec4(tint_symbol_1[zero].f, 1.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = tint_symbol();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
