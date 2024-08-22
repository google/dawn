SKIP: FAILED

struct QuicksortObject {
  int numbers[10];
};

struct main_out {
  float4 frag_color_1;
  float4 gl_Position;
};

struct main_outputs {
  float4 main_out_frag_color_1 : TEXCOORD0;
  float4 main_out_gl_Position : SV_Position;
};

struct main_inputs {
  float4 x_GLF_pos_param : TEXCOORD0;
};


static QuicksortObject obj = (QuicksortObject)0;
static float4 x_GLF_FragCoord = (0.0f).xxxx;
static float4 x_GLF_pos = (0.0f).xxxx;
cbuffer cbuffer_x_33 : register(b0) {
  uint4 x_33[1];
};
cbuffer cbuffer_x_36 : register(b1) {
  uint4 x_36[1];
};
static float4 frag_color = (0.0f).xxxx;
static float4 gl_Position = (0.0f).xxxx;
void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  temp = obj.numbers[i];
  int x_253 = i;
  obj.numbers[x_253] = obj.numbers[j];
  int x_258 = j;
  obj.numbers[x_258] = temp;
}

int performPartition_i1_i1_(inout int l, inout int h) {
  int pivot = 0;
  int i_1 = 0;
  int j_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  int param_3 = 0;
  pivot = obj.numbers[h];
  i_1 = (l - 1);
  j_1 = l;
  {
    while(true) {
      if ((j_1 <= (h - 1))) {
      } else {
        break;
      }
      if ((obj.numbers[j_1] <= pivot)) {
        i_1 = (i_1 + 1);
        param = i_1;
        param_1 = j_1;
        swap_i1_i1_(param, param_1);
      }
      {
        j_1 = (j_1 + 1);
      }
      continue;
    }
  }
  param_2 = (i_1 + 1);
  param_3 = h;
  swap_i1_i1_(param_2, param_3);
  int x_295 = i_1;
  return (x_295 + 1);
}

void quicksort_() {
  int l_1 = 0;
  int h_1 = 0;
  int top = 0;
  int stack[10] = (int[10])0;
  int p = 0;
  int param_4 = 0;
  int param_5 = 0;
  l_1 = 0;
  h_1 = 9;
  top = -1;
  int x_299 = (top + 1);
  top = x_299;
  stack[x_299] = l_1;
  int x_303 = (top + 1);
  top = x_303;
  stack[x_303] = h_1;
  {
    while(true) {
      if ((top >= 0)) {
      } else {
        break;
      }
      int x_313 = top;
      top = (top - 1);
      h_1 = stack[x_313];
      int x_317 = top;
      top = (top - 1);
      l_1 = stack[x_317];
      param_4 = l_1;
      param_5 = h_1;
      int x_323 = performPartition_i1_i1_(param_4, param_5);
      p = x_323;
      if (((p - 1) > l_1)) {
        int x_331 = (top + 1);
        top = x_331;
        stack[x_331] = l_1;
        int x_335 = (top + 1);
        top = x_335;
        stack[x_335] = (p - 1);
      }
      if (((p + 1) < h_1)) {
        int x_346 = (top + 1);
        top = x_346;
        stack[x_346] = (p + 1);
        int x_351 = (top + 1);
        top = x_351;
        stack[x_351] = h_1;
      }
      {
      }
      continue;
    }
  }
}

void main_1() {
  int i_2 = 0;
  float2 uv = (0.0f).xx;
  float3 color = (0.0f).xxx;
  x_GLF_FragCoord = ((x_GLF_pos + float4(1.0f, 1.0f, 0.0f, 0.0f)) * float4(128.0f, 128.0f, 1.0f, 1.0f));
  i_2 = 0;
  {
    while(true) {
      if ((i_2 < 10)) {
      } else {
        break;
      }
      int x_104 = i_2;
      obj.numbers[x_104] = (10 - i_2);
      float v = asfloat(x_33[0u].x);
      if ((v > asfloat(x_33[0u].y))) {
        break;
      }
      int x_115 = i_2;
      obj.numbers[x_115] = (obj.numbers[i_2] * obj.numbers[i_2]);
      {
        i_2 = (i_2 + 1);
      }
      continue;
    }
  }
  quicksort_();
  float2 v_1 = x_GLF_FragCoord.xy;
  uv = (v_1 / asfloat(x_36[0u].xy));
  color = float3(1.0f, 2.0f, 3.0f);
  float v_2 = color.x;
  color[0u] = (v_2 + float(obj.numbers[0]));
  if ((uv.x > 0.25f)) {
    float v_3 = color.x;
    color[0u] = (v_3 + float(obj.numbers[1]));
  }
  if ((uv.x > 0.5f)) {
    float v_4 = color.y;
    color[1u] = (v_4 + float(obj.numbers[2]));
  }
  if ((uv.x > 0.75f)) {
    float v_5 = color.z;
    color[2u] = (v_5 + float(obj.numbers[3]));
  }
  float v_6 = color.y;
  color[1u] = (v_6 + float(obj.numbers[4]));
  if ((uv.y > 0.25f)) {
    float v_7 = color.x;
    color[0u] = (v_7 + float(obj.numbers[5]));
  }
  if ((uv.y > 0.5f)) {
    float v_8 = color.y;
    color[1u] = (v_8 + float(obj.numbers[6]));
  }
  if ((uv.y > 0.75f)) {
    float v_9 = color.z;
    color[2u] = (v_9 + float(obj.numbers[7]));
  }
  float v_10 = color.z;
  color[2u] = (v_10 + float(obj.numbers[8]));
  if ((abs((uv.x - uv.y)) < 0.25f)) {
    float v_11 = color.x;
    color[0u] = (v_11 + float(obj.numbers[9]));
  }
  float3 x_242 = normalize(color);
  frag_color = float4(x_242[0u], x_242[1u], x_242[2u], 1.0f);
  gl_Position = x_GLF_pos;
}

main_out main_inner(float4 x_GLF_pos_param) {
  x_GLF_pos = x_GLF_pos_param;
  main_1();
  main_out v_12 = {frag_color, gl_Position};
  return v_12;
}

main_outputs main(main_inputs inputs) {
  main_out v_13 = main_inner(inputs.x_GLF_pos_param);
  main_out v_14 = v_13;
  main_out v_15 = v_13;
  main_outputs v_16 = {v_14.frag_color_1, v_15.gl_Position};
  return v_16;
}

FXC validation failure:
<scrubbed_path>(139,5-15): error X3511: forced to unroll loop, but unrolling failed.

