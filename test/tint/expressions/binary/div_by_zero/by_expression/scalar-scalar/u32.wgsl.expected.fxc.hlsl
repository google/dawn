uint value_or_one_if_zero_uint(uint value) {
  return value == 0u ? 1u : value;
}

[numthreads(1, 1, 1)]
void f() {
  uint a = 1u;
  uint b = 0u;
  const uint r = (a / value_or_one_if_zero_uint((b + b)));
  return;
}
