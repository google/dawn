#version 310 es


struct a_block {
  int inner;
};

layout(location = 0) uniform a_block v;
void uses_a() {
  int foo = v.inner;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_a();
}
#version 310 es


struct a_block {
  int inner;
};

layout(location = 0) uniform a_block v;
void uses_a() {
  int foo = v.inner;
}
void uses_uses_a() {
  uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_uses_a();
}
#version 310 es


struct b_block {
  int inner;
};

layout(location = 0) uniform b_block v;
void uses_b() {
  int foo = v.inner;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_b();
}
#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
