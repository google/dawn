#version 310 es

float tint_float_modulo(float lhs, float rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}


layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int a = 0;
float b = 0.0f;
int tint_div(int lhs, int rhs) {
  return (lhs / mix(rhs, 1, bool(uint((rhs == 0)) | uint(bool(uint((lhs == (-2147483647 - 1))) & uint((rhs == -1)))))));
}

int tint_mod(int lhs, int rhs) {
  int rhs_or_one = mix(rhs, 1, bool(uint((rhs == 0)) | uint(bool(uint((lhs == (-2147483647 - 1))) & uint((rhs == -1))))));
  if (((uint((lhs | rhs_or_one)) & 2147483648u) != 0u)) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

void foo(int maybe_zero) {
  a = tint_div(a, maybe_zero);
  a = tint_mod(a, maybe_zero);
  b = (b / 0.0f);
  b = tint_float_modulo(b, 0.0f);
  b = (b / float(maybe_zero));
  b = tint_float_modulo(b, float(maybe_zero));
}

