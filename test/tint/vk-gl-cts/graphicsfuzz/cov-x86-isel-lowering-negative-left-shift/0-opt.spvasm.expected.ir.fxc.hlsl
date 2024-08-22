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
  bool x_102 = false;
  A[0] = asfloat(x_6[1u].x);
  A[1] = asfloat(x_6[1u].x);
  i = asint(x_9[0u].x);
  {
    while(true) {
      int v = i;
      if ((v < asint(x_9[3u].x))) {
      } else {
        break;
      }
      j = asint(x_9[0u].x);
      {
        while(true) {
          int v_1 = j;
          if ((v_1 < asint(x_9[2u].x))) {
          } else {
            break;
          }
          int x_66 = j;
          switch(x_66) {
            case 1:
            {
              int x_78 = i;
              A[x_78] = asfloat(x_6[0u].x);
              break;
            }
            case 0:
            {
              if ((-2147483648 < i)) {
                {
                  j = (j + 1);
                }
                continue;
              }
              int x_74 = i;
              A[x_74] = asfloat(x_6[2u].x);
              break;
            }
            default:
            {
              break;
            }
          }
          {
            j = (j + 1);
          }
          continue;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  float v_2 = A[asint(x_9[0u].x)];
  bool x_92 = (v_2 == asfloat(x_6[0u].x));
  x_102 = x_92;
  if (x_92) {
    float v_3 = A[asint(x_9[1u].x)];
    x_101 = (v_3 == asfloat(x_6[0u].x));
    x_102 = x_101;
  }
  if (x_102) {
    float v_4 = float(asint(x_9[1u].x));
    float v_5 = float(asint(x_9[0u].x));
    float v_6 = float(asint(x_9[0u].x));
    x_GLF_color = float4(v_4, v_5, v_6, float(asint(x_9[1u].x)));
  } else {
    x_GLF_color = float4((float(asint(x_9[1u].x))).xxxx);
  }
}

main_out main_inner() {
  main_1();
  main_out v_7 = {x_GLF_color};
  return v_7;
}

main_outputs main() {
  main_out v_8 = main_inner();
  main_outputs v_9 = {v_8.x_GLF_color_1};
  return v_9;
}

FXC validation failure:
C:\src\dawn\Shader@0x000001F51EF5CFE0(55,17-25): error X3708: continue cannot be used in a switch

