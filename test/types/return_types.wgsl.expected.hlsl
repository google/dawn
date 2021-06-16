SKIP: FAILED



Validation Failure:
struct S {
};

bool ret_bool() {
  return false;
}
int ret_i32() {
  return 0;
}
uint ret_u32() {
  return 0u;
}
float ret_f32() {
  return 0.0f;
}
int2 ret_v2i32() {
  return int2(0, 0);
}
uint3 ret_v3u32() {
  return uint3(0u, 0u, 0u);
}
float4 ret_v4f32() {
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
float2x3 ret_m2x3() {
  return float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}
float[4] ret_arr() {
  const float tint_symbol[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  return tint_symbol;
}
S ret_struct() {
  const S tint_symbol_1 = {};
  return tint_symbol_1;
}
[numthreads(1, 1, 1)]
void main() {
  return;
}

tint_rjGobz:28:19: error: brackets are not allowed here; to declare an array, place the brackets after the name
float[4] ret_arr() {
     ~~~          ^
                  [4]

