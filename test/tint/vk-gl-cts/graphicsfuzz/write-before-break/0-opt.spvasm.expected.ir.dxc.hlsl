SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_9 : register(b0) {
  uint4 x_9[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  int idx = 0;
  float4x3 m43 = float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  int ll_1 = 0;
  int GLF_live6rows = 0;
  int z = 0;
  int ll_2 = 0;
  int ctr = 0;
  float4x3 tempm43 = float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  int ll_3 = 0;
  int c = 0;
  int d = 0;
  float GLF_live6sums[9] = (float[9])0;
  idx = 0;
  m43 = float4x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), (0.0f).xxx);
  ll_1 = 0;
  GLF_live6rows = 2;
  {
    while(true) {
      int v = ll_1;
      if ((v >= asint(x_9[0u].x))) {
        x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
        break;
      }
      ll_1 = (ll_1 + 1);
      z = asint(x_9[0u].x);
      ll_2 = 0;
      ctr = 0;
      {
        while(true) {
          if ((ctr < 1)) {
          } else {
            break;
          }
          int v_1 = ll_2;
          if ((v_1 >= asint(x_9[0u].x))) {
            break;
          }
          ll_2 = (ll_2 + 1);
          tempm43 = m43;
          ll_3 = 0;
          c = 0;
          {
            while(true) {
              if ((1 < z)) {
              } else {
                break;
              }
              d = 0;
              int x_29 = c;
              int x_30 = c;
              int x_31 = c;
              int x_32 = d;
              int x_33 = d;
              int x_34 = d;
              float3 v_2 = tempm43[((((x_29 >= 0) & (x_30 < 4))) ? (x_31) : (0))];
              v_2[((((x_32 >= 0) & (x_33 < 3))) ? (x_34) : (0))] = 1.0f;
              {
                c = (c + 1);
              }
              continue;
            }
          }
          int x_117 = ((((idx >= 0) & (idx < 9))) ? (idx) : (0));
          GLF_live6sums[x_117] = (GLF_live6sums[x_117] + m43[ctr].y);
          {
            ctr = (ctr + 1);
          }
          continue;
        }
      }
      idx = (idx + 1);
      {
      }
      continue;
    }
  }
}

main_out main_inner() {
  main_1();
  main_out v_3 = {x_GLF_color};
  return v_3;
}

main_outputs main() {
  main_out v_4 = main_inner();
  main_outputs v_5 = {v_4.x_GLF_color_1};
  return v_5;
}

DXC validation failure:
error: validation errors
hlsl.hlsl:99: error: Loop must have break.
Validation failed.


