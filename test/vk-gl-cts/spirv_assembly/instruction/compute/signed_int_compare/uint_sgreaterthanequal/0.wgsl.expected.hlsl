static uint3 x_2 = uint3(0u, 0u, 0u);
RWByteAddressBuffer x_5 : register(u0, space0);
RWByteAddressBuffer x_6 : register(u1, space0);
RWByteAddressBuffer x_7 : register(u2, space0);

void main_1() {
  const uint x_21 = x_2.x;
  const uint x_23 = x_5.Load((4u * x_21));
  const uint x_25 = x_6.Load((4u * x_21));
  x_7.Store((4u * x_21), asuint(((asint(x_23) >= asint(x_25)) ? 1u : 0u)));
  return;
}

struct tint_symbol_1 {
  uint3 x_2_param : SV_DispatchThreadID;
};

void main_inner(uint3 x_2_param) {
  x_2 = x_2_param;
  main_1();
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x_2_param);
  return;
}
