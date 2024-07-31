
RWByteAddressBuffer buffer : register(u0);
uint foo() {
  uint v_1[4] = {0u, 1u, 2u, 4u};
  return v_1[buffer.Load(0u)];
}

[numthreads(1, 1, 1)]
void main() {
  uint v_2[4] = {0u, 1u, 2u, 4u};
  uint v = v_2[buffer.Load(0u)];
  buffer.Store(0u, (v + foo()));
}

