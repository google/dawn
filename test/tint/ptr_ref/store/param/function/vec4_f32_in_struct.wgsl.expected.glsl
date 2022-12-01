#version 310 es

struct str {
  vec4 i;
};

void func(inout vec4 pointer) {
  pointer = vec4(0.0f);
}

void tint_symbol() {
  str F = str(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  func(F.i);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
