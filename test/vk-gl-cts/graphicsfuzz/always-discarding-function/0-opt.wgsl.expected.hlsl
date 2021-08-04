struct tmp_struct {
  int nmb[1];
};

cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int x_24[1] = (int[1])0;
  bool x_68 = false;
  int x_17 = 0;
  int x_18 = 0;
  int x_19 = 0;
  int x_20 = 0;
  bool x_69 = false;
  float tmp_float = 0.0f;
  float3 color = float3(0.0f, 0.0f, 0.0f);
  while (true) {
    int x_25 = 0;
    float3 x_101 = float3(0.0f, 0.0f, 0.0f);
    bool x_79_phi = false;
    int x_26_phi = 0;
    const float x_75 = asfloat(x_11[0].y);
    tmp_float = x_75;
    const float3 x_76 = float3(x_75, x_75, x_75);
    color = x_76;
    const int tint_symbol_2[1] = {0};
    const tmp_struct tint_symbol_3 = {tint_symbol_2};
    x_24 = tint_symbol_3.nmb;
    x_68 = false;
    x_79_phi = false;
    while (true) {
      int x_21_phi = 0;
      int x_25_phi = 0;
      bool x_93_phi = false;
      const bool x_79 = x_79_phi;
      x_18 = 1;
      x_21_phi = 1;
      while (true) {
        const int x_21 = x_21_phi;
        x_25_phi = 0;
        x_93_phi = x_79;
        if ((x_21 > 10)) {
        } else {
          break;
        }
        const int x_22 = (x_21 - 1);
        x_19 = x_22;
        const int x_23 = x_24[x_22];
        if ((x_23 == 1)) {
          x_68 = true;
          x_17 = 1;
          x_25_phi = 1;
          x_93_phi = true;
          break;
        }
        x_18 = x_22;
        {
          x_21_phi = x_22;
        }
      }
      x_25 = x_25_phi;
      const bool x_93 = x_93_phi;
      x_26_phi = x_25;
      if (x_93) {
        break;
      }
      x_68 = true;
      x_17 = -1;
      x_26_phi = -1;
      break;
      {
        x_79_phi = false;
      }
    }
    const int x_26 = x_26_phi;
    x_20 = x_26;
    if ((x_26 == -1)) {
      discard;
    } else {
      x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
      const float2 x_100 = (float2(x_76.y, x_76.z) + float2(1.0f, 1.0f));
      x_101 = float3(x_76.x, x_100.x, x_100.y);
      color = x_101;
      const float x_103 = asfloat(x_11[0].x);
      if ((x_103 > 1.0f)) {
        x_69 = true;
        break;
      }
    }
    x_GLF_color = float4(x_101.x, x_101.y, x_101.z, 1.0f);
    x_69 = true;
    break;
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
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}

int binarySearch_struct_tmp_struct_i1_1_1_(inout tmp_struct obj) {
  bool x_112 = false;
  int x_16 = 0;
  int one = 0;
  int zero = 0;
  int x_27 = 0;
  bool x_114_phi = false;
  int x_28_phi = 0;
  x_114_phi = false;
  while (true) {
    int x_15_phi = 0;
    int x_27_phi = 0;
    bool x_128_phi = false;
    const bool x_114 = x_114_phi;
    one = 1;
    x_15_phi = 1;
    while (true) {
      const int x_15 = x_15_phi;
      x_27_phi = 0;
      x_128_phi = x_114;
      if ((x_15 > 10)) {
      } else {
        break;
      }
      const int x_13 = (x_15 - 1);
      zero = x_13;
      const int x_14 = obj.nmb[x_13];
      if ((x_14 == 1)) {
        x_112 = true;
        x_16 = 1;
        x_27_phi = 1;
        x_128_phi = true;
        break;
      }
      one = x_13;
      {
        x_15_phi = x_13;
      }
    }
    x_27 = x_27_phi;
    const bool x_128 = x_128_phi;
    x_28_phi = x_27;
    if (x_128) {
      break;
    }
    x_112 = true;
    x_16 = -1;
    x_28_phi = -1;
    break;
    {
      x_114_phi = false;
    }
  }
  return x_28_phi;
}
