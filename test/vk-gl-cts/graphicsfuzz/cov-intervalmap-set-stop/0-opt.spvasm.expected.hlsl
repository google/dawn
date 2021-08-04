static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float3 func_() {
  float2 v = float2(0.0f, 0.0f);
  int i = 0;
  int k = 0;
  v = float2(1.0f, 1.0f);
  i = 0;
  k = 0;
  {
    for(; (k < 2); k = (k + 1)) {
      const float x_94 = v.y;
      if (((x_94 + 1.0f) > 4.0f)) {
        break;
      }
      v.y = 1.0f;
      i = (i + 1);
    }
  }
  if ((i < 10)) {
    return float3(1.0f, 0.0f, 0.0f);
  } else {
    return float3(0.0f, 0.0f, 1.0f);
  }
  return float3(0.0f, 0.0f, 0.0f);
}

void main_1() {
  int j = 0;
  float3 data[2] = (float3[2])0;
  int j_1 = 0;
  bool x_80 = false;
  bool x_81_phi = false;
  j = 0;
  {
    for(; (j < 1); j = (j + 1)) {
      const int x_52 = j;
      const float3 x_53 = func_();
      data[x_52] = x_53;
    }
  }
  j_1 = 0;
  {
    for(; (j_1 < 1); j_1 = (j_1 + 1)) {
      const int x_64 = j_1;
      const float3 x_67 = func_();
      data[((4 * x_64) + 1)] = x_67;
    }
  }
  const float3 x_72 = data[0];
  const bool x_74 = all((x_72 == float3(1.0f, 0.0f, 0.0f)));
  x_81_phi = x_74;
  if (x_74) {
    const float3 x_78 = data[1];
    x_80 = all((x_78 == float3(1.0f, 0.0f, 0.0f)));
    x_81_phi = x_80;
  }
  if (x_81_phi) {
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
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
