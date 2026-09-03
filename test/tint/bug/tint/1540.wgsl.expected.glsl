#version 310 es


struct S {
  bool e;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  bool b = false;
  S v = S(bool((1u & uint(b))));
}
