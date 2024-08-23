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
  int x_10 = 0;
  m44 = float4x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(5.0f, 6.0f, 7.0f, 8.0f), float4(9.0f, 10.0f, 11.0f, 12.0f), float4(13.0f, 14.0f, 15.0f, 16.0f));
  x_10 = 0;
  {
    while(true) {
      int x_11 = 0;
      int x_9 = 0;
      if ((x_10 < 4)) {
      } else {
        break;
      }
      if ((gl_FragCoord.y < 0.0f)) {
        break;
      }
      x_11 = 0;
      {
        while(true) {
          int x_8 = 0;
          if ((x_11 < 4)) {
          } else {
            break;
          }
          {
            float4 v = m44[x_10];
            int v_1 = x_11;
            float v_2 = m44[x_10][x_11];
            v[v_1] = (v_2 + asfloat(x_7[0u].x));
            x_8 = (x_11 + 1);
            x_11 = x_8;
          }
          continue;
        }
      }
      {
        x_9 = (x_10 + 1);
        x_10 = x_9;
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
  x_GLF_color = x_83_1;
}

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out v_3 = {x_GLF_color};
  return v_3;
}

main_outputs main(main_inputs inputs) {
  main_out v_4 = main_inner(float4(inputs.gl_FragCoord_param.xyz, (1.0f / inputs.gl_FragCoord_param[3u])));
  main_outputs v_5 = {v_4.x_GLF_color_1};
  return v_5;
}

FXC validation failure:
<scrubbed_path>(25,5-15): error X3511: unable to unroll loop, loop does not appear to terminate in a timely manner (435 iterations) or unrolled loop is too large, use the [unroll(n)] attribute to force an exact higher number


tint executable returned error: exit status 1
