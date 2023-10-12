RWByteAddressBuffer tint_symbol : register(u0);

void bar(inout float p) {
  const float a = 1.0f;
  const float b = 2.0f;
  bool tint_tmp = (a >= 0.0f);
  if (tint_tmp) {
    tint_tmp = (b >= 0.0f);
  }
  const bool cond = (tint_tmp);
  p = (cond ? b : a);
}

[numthreads(1, 1, 1)]
void foo() {
  float param = 0.0f;
  bar(param);
  tint_symbol.Store(0u, asuint(param));
  return;
}
