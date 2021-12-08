SKIP: FAILED

void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int idx = 0;
  float4x3 m43 = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int ll_1 = 0;
  int GLF_live6rows = 0;
  int z = 0;
  int ll_2 = 0;
  int ctr = 0;
  float4x3 tempm43 = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int ll_3 = 0;
  int c = 0;
  int d = 0;
  float GLF_live6sums[9] = (float[9])0;
  idx = 0;
  m43 = float4x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), float3(0.0f, 0.0f, 0.0f));
  ll_1 = 0;
  GLF_live6rows = 2;
  [loop] while (true) {
    const int x_18 = ll_1;
    const int x_19 = asint(x_9[0].x);
    if ((x_18 >= x_19)) {
      x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
      break;
    }
    ll_1 = (ll_1 + 1);
    const int x_22 = asint(x_9[0].x);
    z = x_22;
    ll_2 = 0;
    ctr = 0;
    {
      [loop] for(; (ctr < 1); ctr = (ctr + 1)) {
        const int x_24 = ll_2;
        const int x_25 = asint(x_9[0].x);
        if ((x_24 >= x_25)) {
          break;
        }
        ll_2 = (ll_2 + 1);
        tempm43 = m43;
        ll_3 = 0;
        c = 0;
        {
          [loop] for(; (1 < z); c = (c + 1)) {
            d = 0;
            set_float3(tempm43[(((c >= 0) & (c < 4)) ? c : 0)], (((d >= 0) & (d < 3)) ? d : 0), 1.0f);
          }
        }
        const int x_117 = (((idx >= 0) & (idx < 9)) ? idx : 0);
        const float x_119 = m43[ctr].y;
        const float x_121 = GLF_live6sums[x_117];
        GLF_live6sums[x_117] = (x_121 + x_119);
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x0000027C57A9E460(51,18-44): error X3531: can't unroll loops marked with loop attribute

