//
// main1
//
#version 310 es

layout(location = 0) uniform uint tint_immediates[1];
void uses_a() {
  int foo = int(tint_immediates[0u]);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_a();
}
//
// main2
//
#version 310 es

layout(location = 0) uniform uint tint_immediates[1];
void uses_a() {
  int foo = int(tint_immediates[0u]);
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

layout(location = 0) uniform uint tint_immediates[1];
void uses_b() {
  int foo = int(tint_immediates[0u]);
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
