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


static float4 gl_FragCoord = (0.0f).xxxx;
cbuffer cbuffer_x_7 : register(b0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  float4x4 m44 = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  int x_10_phi = 0;
  m44 = float4x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(5.0f, 6.0f, 7.0f, 8.0f), float4(9.0f, 10.0f, 11.0f, 12.0f), float4(13.0f, 14.0f, 15.0f, 16.0f));
  x_10_phi = 0;
  {
    while(true) {
      int x_9 = 0;
      int x_11_phi = 0;
      int x_10 = x_10_phi;
      if ((x_10 < 4)) {
      } else {
        break;
      }
      float x_63 = gl_FragCoord.y;
      if ((x_63 < 0.0f)) {
        break;
      }
      x_11_phi = 0;
      {
        while(true) {
          int x_8 = 0;
          int x_11 = x_11_phi;
          if ((x_11 < 4)) {
          } else {
            break;
          }
          {
            float x_72 = asfloat(x_7[0u].x);
            float x_74 = m44[x_10][x_11];
            m44[x_10][x_11] = (x_74 + x_72);
            x_8 = (x_11 + 1);
            x_11_phi = x_8;
          }
          continue;
        }
      }
      {
        x_9 = (x_10 + 1);
        x_10_phi = x_9;
      }
      continue;
    }
  }
  float x_77 = m44[1].y;
  float4 x_79_1 = (0.0f).xxxx;
  x_79_1[0u] = (x_77 - 6.0f);
  float4 x_79 = x_79_1;
  float x_81 = m44[2].z;
  float4 x_83_1 = x_79;
  x_83_1[3u] = (x_81 - 11.0f);
  float4 x_83 = x_83_1;
  x_GLF_color = x_83;
}

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out v = {x_GLF_color};
  return v;
}

main_outputs main(main_inputs inputs) {
  main_out v_1 = main_inner(float4(inputs.gl_FragCoord_param.xyz, (1.0f / inputs.gl_FragCoord_param[3u])));
  main_outputs v_2 = {v_1.x_GLF_color_1};
  return v_2;
}

FXC validation failure:
<scrubbed_path>(25,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
