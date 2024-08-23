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
      bool x_81_phi = false;
      int x_54 = i;
      int x_56 = asint(x_10[3u].x);
      if ((x_54 < x_56)) {
      } else {
        break;
      }
      int x_60 = asint(x_10[0u].x);
      int x_62 = asint(x_10[2u].x);
      float x_64 = asfloat(x_7[0u].x);
      float x_66 = m23[x_60][x_62];
      m23[x_60][x_62] = (x_66 + x_64);
      float x_70 = gl_FragCoord.y;
      float x_72 = asfloat(x_7[0u].x);
      if ((x_70 < x_72)) {
      }
      x_81_phi = true;
      if (true) {
        float x_79 = gl_FragCoord.x;
        x_80 = (x_79 < 0.0f);
        x_81_phi = x_80;
      }
      bool x_81 = x_81_phi;
      if (!(x_81)) {
        break;
      }
      {
        int x_85 = i;
        i = (x_85 + 1);
      }
      continue;
    }
  }
  float2x3 x_87 = m23;
  int x_89 = asint(x_10[1u].x);
  int x_92 = asint(x_10[1u].x);
  int x_95 = asint(x_10[1u].x);
  int x_98 = asint(x_10[1u].x);
  int x_101 = asint(x_10[1u].x);
  int x_104 = asint(x_10[0u].x);
  float v_1 = float(x_89);
  float v_2 = float(x_92);
  float3 v_3 = float3(v_1, v_2, float(x_95));
  float v_4 = float(x_98);
  float v_5 = float(x_101);
  float2x3 x_108 = float2x3(v_3, float3(v_4, v_5, float(x_104)));
  bool v_6 = all((x_87[0u] == x_108[0u]));
  if ((v_6 & all((x_87[1u] == x_108[1u])))) {
    int x_122 = asint(x_10[0u].x);
    int x_125 = asint(x_10[1u].x);
    int x_128 = asint(x_10[1u].x);
    int x_131 = asint(x_10[0u].x);
    float v_7 = float(x_122);
    float v_8 = float(x_125);
    float v_9 = float(x_128);
    x_GLF_color = float4(v_7, v_8, v_9, float(x_131));
  } else {
    int x_135 = asint(x_10[1u].x);
    float x_136 = float(x_135);
    x_GLF_color = float4(x_136, x_136, x_136, x_136);
  }
}

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out v_10 = {x_GLF_color};
  return v_10;
}

main_outputs main(main_inputs inputs) {
  main_out v_11 = main_inner(float4(inputs.gl_FragCoord_param.xyz, (1.0f / inputs.gl_FragCoord_param[3u])));
  main_outputs v_12 = {v_11.x_GLF_color_1};
  return v_12;
}

FXC validation failure:
<scrubbed_path>(43,7-15): error X3500: array reference cannot be used as an l-value; not natively addressable
<scrubbed_path>(30,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
