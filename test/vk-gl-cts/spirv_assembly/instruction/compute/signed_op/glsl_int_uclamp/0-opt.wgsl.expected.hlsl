static uint3 x_3 = uint3(0u, 0u, 0u);
RWByteAddressBuffer x_6 : register(u0, space0);
RWByteAddressBuffer x_7 : register(u1, space0);
RWByteAddressBuffer x_8 : register(u2, space0);
RWByteAddressBuffer x_9 : register(u3, space0);

void main_1() {
  const uint x_26 = x_3.x;
  const int x_28 = asint(x_6.Load((4u * x_26)));
  const int x_30 = asint(x_7.Load((4u * x_26)));
  const int x_32 = asint(x_8.Load((4u * x_26)));
  x_9.Store((4u * x_26), asuint(asint(clamp(asuint(x_28), asuint(x_30), asuint(x_32)))));
  return;
}

struct tint_symbol_1 {
  uint3 x_3_param : SV_DispatchThreadID;
};

void main_inner(uint3 x_3_param) {
  x_3 = x_3_param;
  main_1();
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x_3_param);
  return;
}
