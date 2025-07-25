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


cbuffer cbuffer_x_7 : register(b0) {
  uint4 x_7[1];
};
cbuffer cbuffer_x_10 : register(b1) {
  uint4 x_10[4];
};
static float4 gl_FragCoord = (0.0f).xxxx;
static float4 x_GLF_color = (0.0f).xxxx;
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}

void main_1() {
  float4 data[2] = (float4[2])0;
  int b = int(0);
  int y = int(0);
  int i = int(0);
  float x_42 = asfloat(x_7[0u].x);
  float x_45 = asfloat(x_7[0u].x);
  float4 v = float4(x_42, x_42, x_42, x_42);
  float4 v_1[2] = {v, float4(x_45, x_45, x_45, x_45)};
  data = v_1;
  int x_49 = asint(x_10[1u].x);
  b = x_49;
  float x_51 = gl_FragCoord.y;
  int x_54 = asint(x_10[1u].x);
  float x_56 = gl_FragCoord.y;
  int x_60 = asint(x_10[1u].x);
  int v_2 = tint_f32_to_i32(x_51);
  y = min(max(v_2, (x_54 | tint_f32_to_i32(x_56))), x_60);
  int x_63 = asint(x_10[1u].x);
  i = x_63;
  {
    uint2 tint_loop_idx = (4294967295u).xx;
    while(true) {
      if (all((tint_loop_idx == (0u).xx))) {
        break;
      }
      bool x_82 = false;
      bool x_83_phi = false;
      int x_68 = i;
      int x_70 = asint(x_10[0u].x);
      if ((x_68 < x_70)) {
      } else {
        break;
      }
      int x_73 = b;
      int x_75 = asint(x_10[0u].x);
      bool x_76 = (x_73 > x_75);
      x_83_phi = x_76;
      if (x_76) {
        int x_79 = y;
        int x_81 = asint(x_10[1u].x);
        x_82 = (x_79 > x_81);
        x_83_phi = x_82;
      }
      bool x_83 = x_83_phi;
      if (x_83) {
        break;
      }
      int x_86 = b;
      b = (x_86 + int(1));
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        int x_88 = i;
        i = (x_88 + int(1));
      }
      continue;
    }
  }
  int x_90 = b;
  int x_92 = asint(x_10[0u].x);
  if ((x_90 == x_92)) {
    int x_97 = asint(x_10[2u].x);
    int x_99 = asint(x_10[1u].x);
    int x_101 = asint(x_10[3u].x);
    int x_104 = asint(x_10[1u].x);
    int x_107 = asint(x_10[2u].x);
    int x_110 = asint(x_10[2u].x);
    int x_113 = asint(x_10[1u].x);
    uint v_3 = min(uint(min(max(x_97, x_99), x_101)), 1u);
    float v_4 = float(x_104);
    float v_5 = float(x_107);
    float v_6 = float(x_110);
    data[v_3] = float4(v_4, v_5, v_6, float(x_113));
  }
  int x_118 = asint(x_10[1u].x);
  uint v_7 = min(uint(x_118), 1u);
  float4 x_120 = data[v_7];
  x_GLF_color = float4(x_120.x, x_120.y, x_120.z, x_120.w);
}

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out v_8 = {x_GLF_color};
  return v_8;
}

main_outputs main(main_inputs inputs) {
  main_out v_9 = main_inner(float4(inputs.gl_FragCoord_param.xyz, (1.0f / inputs.gl_FragCoord_param.w)));
  main_outputs v_10 = {v_9.x_GLF_color_1};
  return v_10;
}

