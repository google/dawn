#version 310 es

struct str {
  vec4 i;
};

vec4 func(inout vec4 pointer) {
  return pointer;
}

str P = str(vec4(0.0f, 0.0f, 0.0f, 0.0f));
void tint_symbol() {
  vec4 r = func(P.i);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
