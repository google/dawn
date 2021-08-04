struct StructType {
  float3 col;
  bool4 bbbb;
};

cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  StructType x_33 = (StructType)0;
  int x_38 = 0;
  StructType x_42 = (StructType)0;
  StructType x_33_phi = (StructType)0;
  int x_9_phi = 0;
  StructType x_42_phi = (StructType)0;
  int x_10_phi = 0;
  const StructType tint_symbol_2 = {float3(0.0f, 0.0f, 0.0f), bool4(false, false, false, false)};
  x_33_phi = tint_symbol_2;
  x_9_phi = 0;
  while (true) {
    StructType x_34 = (StructType)0;
    int x_7 = 0;
    x_33 = x_33_phi;
    const int x_9 = x_9_phi;
    const float x_37 = asfloat(x_5[0].y);
    x_38 = int(x_37);
    if ((x_9 < x_38)) {
    } else {
      break;
    }
    {
      x_34 = x_33;
      x_34.col = float3(1.0f, 0.0f, 0.0f);
      x_7 = (x_9 + 1);
      x_33_phi = x_34;
      x_9_phi = x_7;
    }
  }
  x_42_phi = x_33;
  x_10_phi = 0;
  while (true) {
    StructType x_43 = (StructType)0;
    int x_8 = 0;
    x_42 = x_42_phi;
    const int x_10 = x_10_phi;
    if ((x_10 < x_38)) {
    } else {
      break;
    }
    {
      x_43 = x_42;
      x_43.col = float3(1.0f, 0.0f, 0.0f);
      x_8 = (x_10 + 1);
      x_42_phi = x_43;
      x_10_phi = x_8;
    }
  }
  const float3 x_47 = x_42.col;
  x_GLF_color = float4(x_47.x, x_47.y, x_47.z, 1.0f);
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
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
