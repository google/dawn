uint value_or_one_if_zero_uint(uint value) {
  return value == 0u ? 1u : value;
}

[numthreads(1, 1, 1)]
void f() {
  uint3 a = uint3(1u, 2u, 3u);
  uint b = 0u;
  const uint3 r = (a / value_or_one_if_zero_uint((b + b)));
  return;
}
