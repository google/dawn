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
      if ((i < 800)) {
      } else {
        break;
      }
      if ((tint_mod_i32(i, 32) == 0)) {
        result = (result + 0.40000000596046447754f);
      } else {
        float x_149 = thirty_two;
        float v_1 = float(i);
        float v_2 = round(x_149);
        float v_3 = float(i);
        if (((v_1 - (v_2 * floor((v_3 / round(x_149))))) <= 0.00999999977648258209f)) {
          result = (result + 100.0f);
        }
      }
      float v_4 = float(i);
      if ((v_4 >= limit)) {
        float x_163 = result;
        return x_163;
      }
      {
        i = (i + 1);
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
  thirty_two_1 = round((asfloat(x_13[0u].x) / 8.0f));
  param = gl_FragCoord.x;
  param_1 = thirty_two_1;
  float x_69 = compute_value_f1_f1_(param, param_1);
  c[0u] = x_69;
  param_2 = gl_FragCoord.y;
  param_3 = thirty_two_1;
  float x_74 = compute_value_f1_f1_(param_2, param_3);
  c[1u] = x_74;
  float2 v_5 = float2(c.x, c.y);
  float4x2 x_87 = float4x2(v_5, float2(c.z, 1.0f), float2(1.0f, 0.0f), float2(1.0f, 0.0f));
  float v_6 = mul(float3x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f)), c)[0u];
  c[2u] = (v_6 + float3(x_87[0u][0u], x_87[0u][1u], x_87[1u][0u])[1u]);
  i_1 = 0;
  {
    while(true) {
      if ((i_1 < 3)) {
      } else {
        break;
      }
      if ((c[i_1] >= 1.0f)) {
        int x_108 = i_1;
        c[x_108] = (c[i_1] * c[i_1]);
        if ((gl_FragCoord.y < 0.0f)) {
          break;
        }
      }
      {
        i_1 = (i_1 + 1);
      }
      continue;
    }
  }
  float3 x_126 = normalize(abs(c));
  x_GLF_color = float4(x_126[0u], x_126[1u], x_126[2u], 1.0f);
}

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out v_7 = {x_GLF_color};
  return v_7;
}

main_outputs main(main_inputs inputs) {
  main_out v_8 = main_inner(float4(inputs.gl_FragCoord_param.xyz, (1.0f / inputs.gl_FragCoord_param[3u])));
  main_outputs v_9 = {v_8.x_GLF_color_1};
  return v_9;
}

FXC validation failure:
<scrubbed_path>(21,19-25): warning X3556: integer divides may be much slower, try using uints if possible.
<scrubbed_path>(85,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
