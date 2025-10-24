#version 310 es


struct S {
  int i;
  uint u;
  float f;
  bool b;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S v[4] = S[4](S(0, 0u, 0.0f, false), S(0, 0u, 0.0f, false), S(0, 0u, 0.0f, false), S(0, 0u, 0.0f, false));
}
