#version 310 es


struct tint_symbol_1 {
  int tint_symbol;
};

layout(location = 0) uniform tint_symbol_1 v;
void uses_a() {
  int foo = v.tint_symbol;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_a();
}
#version 310 es


struct tint_symbol_1 {
  int tint_symbol;
};

layout(location = 0) uniform tint_symbol_1 v;
void uses_a() {
  int foo = v.tint_symbol;
}
void uses_uses_a() {
  uses_a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_uses_a();
}
#version 310 es


struct tint_symbol_1 {
  int tint_symbol;
};

layout(location = 0) uniform tint_symbol_1 v;
void uses_b() {
  int foo = v.tint_symbol;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uses_b();
}
#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
