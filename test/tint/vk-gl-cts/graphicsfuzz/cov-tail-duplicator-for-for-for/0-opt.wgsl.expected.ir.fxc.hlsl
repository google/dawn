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
  int x_37 = asint(x_7[0u].x);
  i = x_37;
  {
    while(true) {
      int x_42 = i;
      int x_44 = asint(x_7[1u].x);
      if ((x_42 < x_44)) {
      } else {
        break;
      }
      int x_47 = i;
      switch(x_47) {
        case 2:
        {
          int x_83 = i;
          float x_85 = asfloat(x_11[0u].x);
          color[x_83] = x_85;
          break;
        }
        case 1:
        {
          int x_52 = asint(x_7[0u].x);
          j = x_52;
          {
            while(true) {
              int x_57 = i;
              int x_58 = i;
              if ((x_57 > x_58)) {
              } else {
                break;
              }
              int x_62 = asint(x_7[0u].x);
              k = x_62;
              {
                while(true) {
                  int x_67 = k;
                  int x_68 = i;
                  if ((x_67 < x_68)) {
                  } else {
                    break;
                  }
                  int x_71 = k;
                  float x_73 = asfloat(x_11[0u].x);
                  color[x_71] = x_73;
                  {
                    int x_75 = k;
                    k = (x_75 + 1);
                  }
                  continue;
                }
              }
              {
                int x_77 = j;
                j = (x_77 + 1);
              }
              continue;
            }
          }
          int x_79 = i;
          float x_81 = asfloat(x_11[0u].x);
          color[x_79] = x_81;
          break;
        }
        default:
        {
          break;
        }
      }
      {
        int x_87 = i;
        i = (x_87 + 1);
      }
      continue;
    }
  }
  float4 x_89 = color;
  x_GLF_color = x_89;
}

main_out main_inner() {
  main_1();
  main_out v = {x_GLF_color};
  return v;
}

main_outputs main() {
  main_out v_1 = main_inner();
  main_outputs v_2 = {v_1.x_GLF_color_1};
  return v_2;
}

FXC validation failure:
<scrubbed_path>(26,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
