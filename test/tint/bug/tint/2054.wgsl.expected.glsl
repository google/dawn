#version 310 es

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  float inner;
} tint_symbol;

void bar(inout float p) {
  float a = 1.0f;
  float b = 2.0f;
  bool tint_tmp = (a >= 0.0f);
  if (tint_tmp) {
    tint_tmp = (b >= 0.0f);
  }
  bool cond = (tint_tmp);
  p = (cond ? b : a);
}

void foo() {
  float param = 0.0f;
  bar(param);
  tint_symbol.inner = param;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo();
  return;
}
