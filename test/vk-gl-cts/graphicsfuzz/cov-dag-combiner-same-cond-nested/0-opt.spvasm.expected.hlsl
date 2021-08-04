void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f = 0.0f;
  int a = 0;
  int b = 0;
  int c = 0;
  int i = 0;
  float3 v = float3(0.0f, 0.0f, 0.0f);
  const float x_42 = asfloat(x_6[0].x);
  f = x_42;
  a = 1;
  b = 0;
  c = 1;
  i = 0;
  {
    for(; (i < 3); i = (i + 1)) {
      set_float3(v, i, (f + float(i)));
    }
  }
  const float x_59 = asfloat(x_6[0].x);
  if ((x_59 == 1.0f)) {
    while (true) {
      x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
      {
        if (((c & (a | b)) == 0)) {
        } else {
          break;
        }
      }
    }
    const float x_74 = asfloat(x_6[0].x);
    if ((x_74 == 1.0f)) {
      x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
  }
  const float x_79 = v.x;
  const float x_83 = v.y;
  const float x_87 = v.z;
  const float3 x_90 = float3(((x_79 == 1.0f) ? 1.0f : 0.0f), ((x_83 == 2.0f) ? 0.0f : 1.0f), ((x_87 == 3.0f) ? 0.0f : 1.0f));
  x_GLF_color = float4(x_90.x, x_90.y, x_90.z, x_GLF_color.w);
  x_GLF_color.w = 1.0f;
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
