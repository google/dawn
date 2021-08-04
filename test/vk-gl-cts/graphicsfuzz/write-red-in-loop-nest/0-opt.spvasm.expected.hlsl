void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4x3 m43 = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int ll1 = 0;
  int rows = 0;
  int ll4 = 0;
  int ll2 = 0;
  int c = 0;
  float4x3 tempm43 = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int ll3 = 0;
  int d = 0;
  int r = 0;
  float sums[9] = (float[9])0;
  int idx = 0;
  m43 = float4x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), float3(0.0f, 0.0f, 0.0f));
  ll1 = 0;
  rows = 2;
  while (true) {
    if (true) {
    } else {
      break;
    }
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    if ((ll1 >= 5)) {
      break;
    }
    ll1 = (ll1 + 1);
    ll4 = 10;
    ll2 = 0;
    c = 0;
    {
      for(; (c < 1); c = (c + 1)) {
        if ((ll2 >= 0)) {
          break;
        }
        ll2 = (ll2 + 1);
        tempm43 = m43;
        ll3 = 0;
        d = 0;
        {
          for(; (1 < ll4); d = (d + 1)) {
            set_float3(tempm43[(((d >= 0) & (d < 4)) ? d : 0)], (((r >= 0) & (r < 3)) ? r : 0), 1.0f);
          }
        }
        const int x_111 = (((idx >= 0) & (idx < 9)) ? idx : 0);
        const float x_113 = m43[c].y;
        const float x_115 = sums[x_111];
        sums[x_111] = (x_115 + x_113);
      }
    }
    idx = (idx + 1);
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
