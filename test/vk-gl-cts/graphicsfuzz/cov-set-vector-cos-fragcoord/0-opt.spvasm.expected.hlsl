static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2 a = float2(0.0f, 0.0f);
  float3 b = float3(0.0f, 0.0f, 0.0f);
  bool x_105 = false;
  bool x_111 = false;
  bool x_106_phi = false;
  bool x_112_phi = false;
  a = float2(1.0f, 1.0f);
  b = float3(0.0f, 0.0f, 0.0f);
  const float x_52 = gl_FragCoord.y;
  if ((int(x_52) < 40)) {
    b = float3(0.100000001f, 0.100000001f, 0.100000001f);
  } else {
    const float x_59 = gl_FragCoord.y;
    if ((int(x_59) < 60)) {
      b = float3(0.200000003f, 0.200000003f, 0.200000003f);
    } else {
      const float x_66 = gl_FragCoord.y;
      if ((x_66 < 80.0f)) {
        const float x_72 = a.x;
        const float x_74 = asfloat(x_8[0].x);
        b = (cos((float3(x_72, x_72, x_72) + float3(x_74, x_74, x_74))) + float3(0.01f, 0.01f, 0.01f));
      } else {
        const float x_82 = gl_FragCoord.y;
        if ((int(x_82) < 100)) {
          const float x_89 = asfloat(x_8[0].x);
          b = cos(float3(x_89, x_89, x_89));
        } else {
          const float x_93 = gl_FragCoord.y;
          if ((int(x_93) < 500)) {
            b = float3(0.540302277f, 0.540302277f, -0.99996084f);
          }
        }
      }
    }
  }
  const float x_99 = b.x;
  const bool x_100 = (x_99 < 1.019999981f);
  x_106_phi = x_100;
  if (x_100) {
    const float x_104 = b.y;
    x_105 = (x_104 < 1.019999981f);
    x_106_phi = x_105;
  }
  const bool x_106 = x_106_phi;
  x_112_phi = x_106;
  if (x_106) {
    const float x_110 = b.z;
    x_111 = (x_110 < 1.019999981f);
    x_112_phi = x_111;
  }
  if (x_112_phi) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
