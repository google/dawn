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


cbuffer cbuffer_x_7 : register(b1) {
  uint4 x_7[2];
};
cbuffer cbuffer_x_10 : register(b0) {
  uint4 x_10[4];
};
static float4 gl_FragCoord = (0.0f).xxxx;
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  float2x3 m23 = float2x3((0.0f).xxx, (0.0f).xxx);
  int i = 0;
  float x_46 = asfloat(x_7[1u].x);
  float3 v = float3(x_46, 0.0f, 0.0f);
  m23 = float2x3(v, float3(0.0f, x_46, 0.0f));
  i = 1;
  {
    while(true) {
      bool x_80 = false;
      bool x_81 = false;
      int v_1 = i;
      if ((v_1 < asint(x_10[3u].x))) {
      } else {
        break;
      }
      int x_60 = asint(x_10[0u].x);
      int x_62 = asint(x_10[2u].x);
      float v_2 = m23[x_60][x_62];
      m23[x_60][x_62] = (v_2 + asfloat(x_7[0u].x));
      float v_3 = gl_FragCoord.y;
      if ((v_3 < asfloat(x_7[0u].x))) {
      }
      x_81 = true;
      if (true) {
        x_80 = (gl_FragCoord.x < 0.0f);
        x_81 = x_80;
      }
      if (!(x_81)) {
        break;
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  float v_4 = float(asint(x_10[1u].x));
  float v_5 = float(asint(x_10[1u].x));
  float3 v_6 = float3(v_4, v_5, float(asint(x_10[1u].x)));
  float v_7 = float(asint(x_10[1u].x));
  float v_8 = float(asint(x_10[1u].x));
  float2x3 x_108 = float2x3(v_6, float3(v_7, v_8, float(asint(x_10[0u].x))));
  bool v_9 = all((m23[0u] == x_108[0u]));
  if ((v_9 & all((m23[1u] == x_108[1u])))) {
    float v_10 = float(asint(x_10[0u].x));
    float v_11 = float(asint(x_10[1u].x));
    float v_12 = float(asint(x_10[1u].x));
    x_GLF_color = float4(v_10, v_11, v_12, float(asint(x_10[0u].x)));
  } else {
    x_GLF_color = float4((float(asint(x_10[1u].x))).xxxx);
  }
}

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out v_13 = {x_GLF_color};
  return v_13;
}

main_outputs main(main_inputs inputs) {
  main_out v_14 = main_inner(float4(inputs.gl_FragCoord_param.xyz, (1.0f / inputs.gl_FragCoord_param[3u])));
  main_outputs v_15 = {v_14.x_GLF_color_1};
  return v_15;
}

FXC validation failure:
<scrubbed_path>(41,7-15): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(30,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
