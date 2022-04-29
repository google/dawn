SKIP: FAILED - see https://github.com/microsoft/DirectXShaderCompiler/issues/4422

cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  bool x_36 = false;
  bool x_37 = false;
  int x_7 = 0;
  bool x_38 = false;
  float3 color = float3(0.0f, 0.0f, 0.0f);
  bool x_40 = false;
  float3 x_43 = float3(0.0f, 0.0f, 0.0f);
  bool x_40_phi = false;
  float3 x_42_phi = float3(0.0f, 0.0f, 0.0f);
  bool x_56_phi = false;
  bool x_58_phi = false;
  x_40_phi = false;
  x_42_phi = float3(0.0f, 0.0f, 0.0f);
  [loop] while (true) {
    float3 x_43_phi = float3(0.0f, 0.0f, 0.0f);
    x_40 = x_40_phi;
    const float3 x_42 = x_42_phi;
    const float x_47 = asfloat(x_5[0].y);
    x_43_phi = x_42;
    if ((x_47 < 0.0f)) {
      color = float3(1.0f, 1.0f, 1.0f);
      x_43_phi = float3(1.0f, 1.0f, 1.0f);
    }
    x_43 = x_43_phi;
    {
      x_40_phi = x_40;
      x_42_phi = x_43;
      if (false) {
      } else {
        break;
      }
    }
  }
  x_36 = false;
  x_56_phi = x_40;
  x_58_phi = false;
  [loop] while (true) {
    bool x_62 = false;
    bool x_62_phi = false;
    bool x_64_phi = false;
    int x_65_phi = 0;
    bool x_70_phi = false;
    bool x_71_phi = false;
    const bool x_56 = x_56_phi;
    const bool x_58 = x_58_phi;
    x_7 = 0;
    x_62_phi = x_56;
    x_64_phi = false;
    x_65_phi = 0;
    [loop] while (true) {
      x_62 = x_62_phi;
      const bool x_64 = x_64_phi;
      const int x_65 = x_65_phi;
      const bool x_68 = (0 < 1);
      x_70_phi = x_62;
      x_71_phi = false;
      if (true) {
      } else {
        break;
      }
      x_36 = true;
      x_37 = true;
      x_70_phi = true;
      x_71_phi = true;
      break;
      {
        x_62_phi = false;
        x_64_phi = false;
        x_65_phi = 0;
      }
    }
    const bool x_70 = x_70_phi;
    const bool x_71 = x_71_phi;
    if (true) {
      break;
    }
    x_36 = true;
    break;
    {
      x_56_phi = false;
      x_58_phi = false;
    }
  }
  x_38 = true;
  const float x_73 = (true ? 1.0f : 0.0f);
  x_GLF_color = (float4(x_43.x, x_43.y, x_43.z, 1.0f) + float4(x_73, x_73, x_73, x_73));
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
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
Internal compiler error: access violation. Attempted to read from address 0x0000000000000048

