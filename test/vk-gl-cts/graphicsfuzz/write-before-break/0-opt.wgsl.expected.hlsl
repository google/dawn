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
  while (true) {
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
      for(; (ctr < 1); ctr = (ctr + 1)) {
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
          for(; (1 < z); c = (c + 1)) {
            d = 0;
            bool tint_tmp = (c >= 0);
            if (tint_tmp) {
              tint_tmp = (c < 4);
            }
            bool tint_tmp_1 = (d >= 0);
            if (tint_tmp_1) {
              tint_tmp_1 = (d < 3);
            }
            set_float3(tempm43[((tint_tmp) ? c : 0)], ((tint_tmp_1) ? d : 0), 1.0f);
          }
        }
        bool tint_tmp_2 = (idx >= 0);
        if (tint_tmp_2) {
          tint_tmp_2 = (idx < 9);
        }
        const int x_117 = ((tint_tmp_2) ? idx : 0);
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

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  const tint_symbol tint_symbol_3 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_3;
}
C:\src\tint\test\Shader@0x00000232DC6E3D00(61,24-52): warning X3550: array reference cannot be used as an l-value; not natively addressable, forcing loop to unroll
C:\src\tint\test\Shader@0x00000232DC6E3D00(51,11-37): error X3511: unable to unroll loop, loop does not appear to terminate in a timely manner (1024 iterations)
C:\src\tint\test\Shader@0x00000232DC6E3D00(40,7-39): error X3511: forced to unroll loop, but unrolling failed.
C:\src\tint\test\Shader@0x00000232DC6E3D00(27,3-14): error X3511: forced to unroll loop, but unrolling failed.

