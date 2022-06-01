struct S {
  float a;
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
  return (0).xx;
}

uint3 ret_v3u32() {
  return (0u).xxx;
}

float4 ret_v4f32() {
  return (0.0f).xxxx;
}

float2x3 ret_m2x3() {
  return float2x3((0.0f).xxx, (0.0f).xxx);
}

typedef float ret_arr_ret[4];
ret_arr_ret ret_arr() {
  const float tint_symbol[4] = (float[4])0;
  return tint_symbol;
}

S ret_struct() {
  const S tint_symbol_1 = (S)0;
  return tint_symbol_1;
}

[numthreads(1, 1, 1)]
void main() {
  return;
}
