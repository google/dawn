//
// main1
//
#version 310 es


struct tint_immediate_struct {
  int inner;
};

layout(location = 0) uniform tint_immediate_struct tint_immediates;
void uses_a() {
  int foo = tint_immediates.inner;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_a();
}
//
// main2
//
#version 310 es


struct tint_immediate_struct {
  int inner;
};

layout(location = 0) uniform tint_immediate_struct tint_immediates;
void uses_a() {
  int foo = tint_immediates.inner;
}
void uses_uses_a() {
  uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_uses_a();
}
//
// main3
//
#version 310 es


struct tint_immediate_struct {
  int inner;
};

layout(location = 0) uniform tint_immediate_struct tint_immediates;
void uses_b() {
  int foo = tint_immediates.inner;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_b();
}
//
// main4
//
#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
