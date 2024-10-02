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
  int idx = int(0);
  float4x3 m43 = float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  int ll_1 = int(0);
  int GLF_live6rows = int(0);
  int z = int(0);
  int ll_2 = int(0);
  int ctr = int(0);
  float4x3 tempm43 = float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  int ll_3 = int(0);
  int c = int(0);
  int d = int(0);
  float GLF_live6sums[9] = (float[9])0;
  idx = int(0);
  m43 = float4x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), (0.0f).xxx);
  ll_1 = int(0);
  GLF_live6rows = int(2);
  {
    while(true) {
      int v = ll_1;
      if ((v >= asint(x_9[0u].x))) {
        x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
        break;
      }
      ll_1 = (ll_1 + int(1));
      z = asint(x_9[0u].x);
      ll_2 = int(0);
      ctr = int(0);
      {
        while(true) {
          if ((ctr < int(1))) {
          } else {
            break;
          }
          int v_1 = ll_2;
          if ((v_1 >= asint(x_9[0u].x))) {
            break;
          }
          ll_2 = (ll_2 + int(1));
          tempm43 = m43;
          ll_3 = int(0);
          c = int(0);
          {
            while(true) {
              if ((int(1) < z)) {
              } else {
                break;
              }
              d = int(0);
              int x_29 = c;
              int x_30 = c;
              int x_31 = c;
              int x_32 = d;
              int x_33 = d;
              int x_34 = d;
              int v_2 = ((((x_29 >= int(0)) & (x_30 < int(4)))) ? (x_31) : (int(0)));
              tempm43[v_2][((((x_32 >= int(0)) & (x_33 < int(3)))) ? (x_34) : (int(0)))] = 1.0f;
              {
                c = (c + int(1));
              }
              continue;
            }
          }
          int x_117 = ((((idx >= int(0)) & (idx < int(9)))) ? (idx) : (int(0)));
          float v_3 = GLF_live6sums[x_117];
          int v_4 = ctr;
          GLF_live6sums[x_117] = (v_3 + m43[v_4].y);
          {
            ctr = (ctr + int(1));
          }
          continue;
        }
      }
      idx = (idx + int(1));
      {
      }
      continue;
    }
  }
}

main_out main_inner() {
  main_1();
  main_out v_5 = {x_GLF_color};
  return v_5;
}

main_outputs main() {
  main_out v_6 = main_inner();
  main_outputs v_7 = {v_6.x_GLF_color_1};
  return v_7;
}

FXC validation failure:
<scrubbed_path>(57,13-23): error X3511: forced to unroll loop, but unrolling failed.
<scrubbed_path>(43,9-19): error X3511: forced to unroll loop, but unrolling failed.
<scrubbed_path>(32,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
