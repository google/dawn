SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_6 : register(b1) {
  uint4 x_6[3];
};
cbuffer cbuffer_x_9 : register(b0) {
  uint4 x_9[4];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  float A[2] = (float[2])0;
  int i = 0;
  int j = 0;
  bool x_101 = false;
  bool x_102_phi = false;
  float x_39 = asfloat(x_6[1u].x);
  A[0] = x_39;
  float x_42 = asfloat(x_6[1u].x);
  A[1] = x_42;
  int x_45 = asint(x_9[0u].x);
  i = x_45;
  {
    while(true) {
      int x_50 = i;
      int x_52 = asint(x_9[3u].x);
      if ((x_50 < x_52)) {
      } else {
        break;
      }
      int x_56 = asint(x_9[0u].x);
      j = x_56;
      {
        while(true) {
          int x_61 = j;
          int x_63 = asint(x_9[2u].x);
          if ((x_61 < x_63)) {
          } else {
            break;
          }
          int x_66 = j;
          switch(x_66) {
            case 1:
            {
              int x_78 = i;
              float x_80 = asfloat(x_6[0u].x);
              A[x_78] = x_80;
              break;
            }
            case 0:
            {
              int x_70 = i;
              if ((-2147483648 < x_70)) {
                {
                  int x_82 = j;
                  j = (x_82 + 1);
                }
                continue;
              }
              int x_74 = i;
              float x_76 = asfloat(x_6[2u].x);
              A[x_74] = x_76;
              break;
            }
            default:
            {
              break;
            }
          }
          {
            int x_82 = j;
            j = (x_82 + 1);
          }
          continue;
        }
      }
      {
        int x_84 = i;
        i = (x_84 + 1);
      }
      continue;
    }
  }
  int x_87 = asint(x_9[0u].x);
  float x_89 = A[x_87];
  float x_91 = asfloat(x_6[0u].x);
  bool x_92 = (x_89 == x_91);
  x_102_phi = x_92;
  if (x_92) {
    int x_96 = asint(x_9[1u].x);
    float x_98 = A[x_96];
    float x_100 = asfloat(x_6[0u].x);
    x_101 = (x_98 == x_100);
    x_102_phi = x_101;
  }
  bool x_102 = x_102_phi;
  if (x_102) {
    int x_107 = asint(x_9[1u].x);
    int x_110 = asint(x_9[0u].x);
    int x_113 = asint(x_9[0u].x);
    int x_116 = asint(x_9[1u].x);
    float v = float(x_107);
    float v_1 = float(x_110);
    float v_2 = float(x_113);
    x_GLF_color = float4(v, v_1, v_2, float(x_116));
  } else {
    int x_120 = asint(x_9[1u].x);
    float x_121 = float(x_120);
    x_GLF_color = float4(x_121, x_121, x_121, x_121);
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

FXC validation failure:
C:\src\dawn\Shader@0x0000022CF32B5050(64,17-25): error X3708: continue cannot be used in a switch

