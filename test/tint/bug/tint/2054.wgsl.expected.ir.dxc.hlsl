
RWByteAddressBuffer tint_symbol : register(u0);
void bar(inout float p) {
  float a = 1.0f;
  float b = 2.0f;
  bool v = false;
  if ((a >= 0.0f)) {
    v = (b >= 0.0f);
  } else {
    v = false;
  }
  bool cond = v;
  p = ((cond) ? (b) : (a));
}

[numthreads(1, 1, 1)]
void foo() {
  float param = 0.0f;
  bar(param);
  tint_symbol.Store(0u, asuint(param));
}

