SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};

struct main_inputs {
  float4 gl_FragCoord_param : SV_Position;
};


cbuffer cbuffer_x_9 : register(b0) {
  uint4 x_9[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
static float4 gl_FragCoord = (0.0f).xxxx;
static bool continue_execution = true;
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (-2147483648))) : (2147483647));
}

void main_1() {
  float2 v = (0.0f).xx;
  float floats[9] = (float[9])0;
  int one = 0;
  int i = 0;
  bool alwaysFalse = false;
  v = (0.0f).xx;
  floats[1] = 0.0f;
  float x_46 = asfloat(x_9[0u].y);
  one = tint_f32_to_i32(x_46);
  {
    while(true) {
      i = 0;
      {
        while(true) {
          int x_56 = i;
          int x_57 = one;
          if ((x_56 < x_57)) {
          } else {
            break;
          }
          int x_60 = i;
          if ((x_60 == 0)) {
            float x_65 = asfloat(x_9[0u].x);
            float x_67 = asfloat(x_9[0u].y);
            alwaysFalse = (x_65 > x_67);
            bool x_69 = alwaysFalse;
            if (!(x_69)) {
              int x_73 = one;
              floats[x_73] = 1.0f;
              x_GLF_color = float4(1.0f, 1.0f, 0.0f, 1.0f);
            }
            int x_75 = one;
            v[x_75] = 1.0f;
            bool x_77 = alwaysFalse;
            if (x_77) {
              continue_execution = false;
            }
            float x_81 = asfloat(x_9[0u].y);
            if ((x_81 < 0.0f)) {
              x_GLF_color = float4(0.0f, 1.0f, 0.0f, 1.0f);
            }
          }
          {
            int x_85 = i;
            i = (x_85 + 1);
          }
          continue;
        }
      }
      {
        int x_87 = one;
        if (!((x_87 < 0))) { break; }
      }
      continue;
    }
  }
  bool x_102 = false;
  bool x_103_phi = false;
  float x_90 = gl_FragCoord.y;
  if ((x_90 >= 0.0f)) {
    float x_96 = v.y;
    bool x_97 = (x_96 == 1.0f);
    x_103_phi = x_97;
    if (x_97) {
      float x_101 = floats[1];
      x_102 = (x_101 == 1.0f);
      x_103_phi = x_102;
    }
    bool x_103 = x_103_phi;
    if (x_103) {
      x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    }
  } else {
    x_GLF_color = (0.0f).xxxx;
  }
}

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out v_1 = {x_GLF_color};
  return v_1;
}

main_outputs main(main_inputs inputs) {
  main_out v_2 = main_inner(float4(inputs.gl_FragCoord_param.xyz, (1.0f / inputs.gl_FragCoord_param[3u])));
  main_outputs v_3 = {v_2.x_GLF_color_1};
  if (!(continue_execution)) {
    discard;
  }
  main_outputs v_4 = v_3;
  return v_4;
}

FXC validation failure:
<scrubbed_path>(57,13-19): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(38,9-19): error X3511: forced to unroll loop, but unrolling failed.
<scrubbed_path>(35,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
