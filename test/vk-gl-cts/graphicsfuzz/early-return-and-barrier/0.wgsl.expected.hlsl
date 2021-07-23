RWByteAddressBuffer x_4 : register(u0, space0);

void main_1() {
  float2x2 x_30 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 x_30_phi = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  int x_32_phi = 0;
  x_4.Store(0u, asuint(42));
  x_30_phi = float2x2(float2(0.0f, 0.0f), float2(0.0f, 0.0f));
  x_32_phi = 1;
  while (true) {
    int x_33 = 0;
    x_30 = x_30_phi;
    const int x_32 = x_32_phi;
    if ((x_32 > 0)) {
    } else {
      break;
    }
    {
      x_33 = (x_32 - 1);
      x_30_phi = float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f));
      x_32_phi = x_33;
    }
  }
  const float2 x_41 = mul(float2x2((x_30[0u] - float2(1.0f, 0.0f)), (x_30[1u] - float2(0.0f, 1.0f))), float2(1.0f, 1.0f));
  while (true) {
    if ((1.0f < x_41.x)) {
      break;
    }
    GroupMemoryBarrierWithGroupSync();
    break;
  }
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
