SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_7 : register(b0) {
  uint4 x_7[2];
};
cbuffer cbuffer_x_11 : register(b1) {
  uint4 x_11[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  float4 color = (0.0f).xxxx;
  int i = 0;
  int j = 0;
  int k = 0;
  color = (1.0f).xxxx;
  i = asint(x_7[0u].x);
  {
    while(true) {
      int v = i;
      if ((v < asint(x_7[1u].x))) {
      } else {
        break;
      }
      int x_47 = i;
      switch(x_47) {
        case 2:
        {
          int x_83 = i;
          color[x_83] = asfloat(x_11[0u].x);
          break;
        }
        case 1:
        {
          j = asint(x_7[0u].x);
          {
            while(true) {
              if ((i > i)) {
              } else {
                break;
              }
              k = asint(x_7[0u].x);
              {
                while(true) {
                  if ((k < i)) {
                  } else {
                    break;
                  }
                  int x_71 = k;
                  color[x_71] = asfloat(x_11[0u].x);
                  {
                    k = (k + 1);
                  }
                  continue;
                }
              }
              {
                j = (j + 1);
              }
              continue;
            }
          }
          int x_79 = i;
          color[x_79] = asfloat(x_11[0u].x);
          break;
        }
        default:
        {
          break;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  x_GLF_color = color;
}

main_out main_inner() {
  main_1();
  main_out v_1 = {x_GLF_color};
  return v_1;
}

main_outputs main() {
  main_out v_2 = main_inner();
  main_outputs v_3 = {v_2.x_GLF_color_1};
  return v_3;
}

FXC validation failure:
<scrubbed_path>(25,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
