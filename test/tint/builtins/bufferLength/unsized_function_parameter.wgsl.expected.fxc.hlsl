
RWByteAddressBuffer v : register(u0);
RWByteAddressBuffer v_1 : register(u1);
void foo() {
  uint v_2 = 0u;
  v.GetDimensions(v_2);
  v_1.Store(0u, v_2);
}

[numthreads(1, 1, 1)]
void main() {
  foo();
}

