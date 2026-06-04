
static int a = int(0);
static float b = 0.0f;
int tint_mod_i32(int lhs, int rhs) {
  int v = select(((rhs == int(0)) | ((lhs == int(-2147483648)) & (rhs == int(-1)))), int(1), rhs);
  return asint((asuint(lhs) - asuint(asint((asuint((lhs / v)) * asuint(v))))));
}

int tint_div_i32(int lhs, int rhs) {
  return (lhs / select(((rhs == int(0)) | ((lhs == int(-2147483648)) & (rhs == int(-1)))), int(1), rhs));
}

void foo(int maybe_zero) {
  a = tint_div_i32(a, maybe_zero);
  a = tint_mod_i32(a, maybe_zero);
  b = (b / 0.0f);
  float v_1 = b;
  float v_2 = (v_1 / 0.0f);
  b = (v_1 - (trunc(v_2) * 0.0f));
  float v_3 = b;
  b = (v_3 / float(maybe_zero));
  float v_4 = b;
  float v_5 = float(maybe_zero);
  float v_6 = (v_4 / v_5);
  b = (v_4 - (trunc(v_6) * v_5));
}

[numthreads(1, 1, 1)]
void main() {
  foo(int(0));
}

