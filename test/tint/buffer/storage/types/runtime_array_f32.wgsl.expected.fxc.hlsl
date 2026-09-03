
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
[numthreads(1, 1, 1)]
void main() {
  uint v_2 = 0u;
  v_1.GetDimensions(v_2);
  uint v_3 = 0u;
  v.GetDimensions(v_3);
  v_1.Store((0u + (min(0u, ((v_2 / 4u) - 1u)) * 4u)), asuint(asfloat(v.Load((0u + (min(0u, ((v_3 / 4u) - 1u)) * 4u))))));
}

