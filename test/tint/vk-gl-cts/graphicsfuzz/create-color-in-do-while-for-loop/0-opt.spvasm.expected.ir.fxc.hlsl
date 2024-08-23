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
  one = tint_f32_to_i32(asfloat(x_9[0u].y));
  {
    while(true) {
      i = 0;
      {
        while(true) {
          if ((i < one)) {
          } else {
            break;
          }
          if ((i == 0)) {
            float v_1 = asfloat(x_9[0u].x);
            alwaysFalse = (v_1 > asfloat(x_9[0u].y));
            if (!(alwaysFalse)) {
              int x_73 = one;
              floats[x_73] = 1.0f;
              x_GLF_color = float4(1.0f, 1.0f, 0.0f, 1.0f);
            }
            int x_75 = one;
            v[x_75] = 1.0f;
            if (alwaysFalse) {
              continue_execution = false;
            }
            if ((asfloat(x_9[0u].y) < 0.0f)) {
              x_GLF_color = float4(0.0f, 1.0f, 0.0f, 1.0f);
            }
          }
          {
            i = (i + 1);
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
  bool x_103 = false;
  if ((gl_FragCoord.y >= 0.0f)) {
    bool x_97 = (v.y == 1.0f);
    x_103 = x_97;
    if (x_97) {
      x_102 = (floats[1] == 1.0f);
      x_103 = x_102;
    }
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
  main_out v_2 = {x_GLF_color};
  return v_2;
}

main_outputs main(main_inputs inputs) {
  main_out v_3 = main_inner(float4(inputs.gl_FragCoord_param.xyz, (1.0f / inputs.gl_FragCoord_param[3u])));
  main_outputs v_4 = {v_3.x_GLF_color_1};
  if (!(continue_execution)) {
    discard;
  }
  main_outputs v_5 = v_4;
  return v_5;
}

FXC validation failure:
<scrubbed_path>(51,13-19): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(37,9-19): error X3511: forced to unroll loop, but unrolling failed.
<scrubbed_path>(34,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
