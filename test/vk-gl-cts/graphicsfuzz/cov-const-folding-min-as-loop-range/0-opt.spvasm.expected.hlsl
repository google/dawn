cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int highSigned = 0;
  uint highUnsigned = 0u;
  int i = 0;
  int data[2] = (int[2])0;
  uint i_1 = 0u;
  bool x_78 = false;
  bool x_79_phi = false;
  highSigned = 1;
  highUnsigned = 2u;
  i = 0;
  while (true) {
    const int x_42 = i;
    const int x_43 = highSigned;
    const int x_46 = asint(x_8[0].x);
    if ((x_42 < (min(10, x_43) + x_46))) {
    } else {
      break;
    }
    data[i] = 5;
    {
      i = (i + 1);
    }
  }
  i_1 = 1u;
  while (true) {
    const uint x_58 = i_1;
    const uint x_59 = highUnsigned;
    const int x_62 = asint(x_8[0].x);
    if ((x_58 < (min(10u, x_59) + asuint(x_62)))) {
    } else {
      break;
    }
    data[i_1] = 6;
    {
      i_1 = (i_1 + asuint(1));
    }
  }
  const int x_72 = data[0];
  const bool x_73 = (x_72 == 5);
  x_79_phi = x_73;
  if (x_73) {
    const int x_77 = data[1];
    x_78 = (x_77 == 6);
    x_79_phi = x_78;
  }
  if (x_79_phi) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
