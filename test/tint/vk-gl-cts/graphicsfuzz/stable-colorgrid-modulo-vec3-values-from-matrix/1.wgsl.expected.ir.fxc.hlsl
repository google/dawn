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


cbuffer cbuffer_x_13 : register(b0) {
  uint4 x_13[1];
};
static float4 gl_FragCoord = (0.0f).xxxx;
static float4 x_GLF_color = (0.0f).xxxx;
int tint_mod_i32(int lhs, int rhs) {
  int v = ((((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v) * v));
}

float compute_value_f1_f1_(inout float limit, inout float thirty_two) {
  float result = 0.0f;
  int i = 0;
  result = -0.5f;
  i = 1;
  {
    while(true) {
      int x_136 = i;
      if ((x_136 < 800)) {
      } else {
        break;
      }
      int x_139 = i;
      if ((tint_mod_i32(x_139, 32) == 0)) {
        float x_145 = result;
        result = (x_145 + 0.40000000596046447754f);
      } else {
        int x_147 = i;
        float x_149 = thirty_two;
        float v_1 = float(x_147);
        float v_2 = round(x_149);
        float v_3 = float(x_147);
        if (((v_1 - (v_2 * floor((v_3 / round(x_149))))) <= 0.00999999977648258209f)) {
          float x_155 = result;
          result = (x_155 + 100.0f);
        }
      }
      int x_157 = i;
      float x_159 = limit;
      if ((float(x_157) >= x_159)) {
        float x_163 = result;
        return x_163;
      }
      {
        int x_164 = i;
        i = (x_164 + 1);
      }
      continue;
    }
  }
  float x_166 = result;
  return x_166;
}

void main_1() {
  float3 c = (0.0f).xxx;
  float thirty_two_1 = 0.0f;
  float param = 0.0f;
  float param_1 = 0.0f;
  float param_2 = 0.0f;
  float param_3 = 0.0f;
  int i_1 = 0;
  c = float3(7.0f, 8.0f, 9.0f);
  float x_63 = asfloat(x_13[0u].x);
  thirty_two_1 = round((x_63 / 8.0f));
  float x_67 = gl_FragCoord.x;
  param = x_67;
  float x_68 = thirty_two_1;
  param_1 = x_68;
  float x_69 = compute_value_f1_f1_(param, param_1);
  c[0u] = x_69;
  float x_72 = gl_FragCoord.y;
  param_2 = x_72;
  float x_73 = thirty_two_1;
  param_3 = x_73;
  float x_74 = compute_value_f1_f1_(param_2, param_3);
  c[1u] = x_74;
  float3 x_76 = c;
  float3 x_79 = c;
  float2 v_4 = float2(x_79[0u], x_79[1u]);
  float4x2 x_87 = float4x2(v_4, float2(x_79[2u], 1.0f), float2(1.0f, 0.0f), float2(1.0f, 0.0f));
  float v_5 = mul(float3x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f)), x_76)[0u];
  c[2u] = (v_5 + float3(x_87[0u][0u], x_87[0u][1u], x_87[1u][0u])[1u]);
  i_1 = 0;
  {
    while(true) {
      int x_99 = i_1;
      if ((x_99 < 3)) {
      } else {
        break;
      }
      int x_102 = i_1;
      float x_104 = c[x_102];
      if ((x_104 >= 1.0f)) {
        int x_108 = i_1;
        int x_109 = i_1;
        float x_111 = c[x_109];
        int x_112 = i_1;
        float x_114 = c[x_112];
        c[x_108] = (x_111 * x_114);
        float x_118 = gl_FragCoord.y;
        if ((x_118 < 0.0f)) {
          break;
        }
      }
      {
        int x_122 = i_1;
        i_1 = (x_122 + 1);
      }
      continue;
    }
  }
  float3 x_124 = c;
  float3 x_126 = normalize(abs(x_124));
  x_GLF_color = float4(x_126[0u], x_126[1u], x_126[2u], 1.0f);
}

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out v_6 = {x_GLF_color};
  return v_6;
}

main_outputs main(main_inputs inputs) {
  main_out v_7 = main_inner(float4(inputs.gl_FragCoord_param.xyz, (1.0f / inputs.gl_FragCoord_param[3u])));
  main_outputs v_8 = {v_7.x_GLF_color_1};
  return v_8;
}

FXC validation failure:
<scrubbed_path>(21,19-25): warning X3556: integer divides may be much slower, try using uints if possible.
<scrubbed_path>(99,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
