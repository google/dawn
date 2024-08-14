SKIP: FAILED

struct QuicksortObject {
  int numbers[10];
};

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};

struct main_inputs {
  float4 gl_FragCoord_param : SV_Position;
};


static QuicksortObject obj = (QuicksortObject)0;
static float4 gl_FragCoord = (0.0f).xxxx;
cbuffer cbuffer_x_34 : register(b0) {
  uint4 x_34[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  temp = obj.numbers[i];
  int x_233 = i;
  obj.numbers[x_233] = obj.numbers[j];
  int x_238 = j;
  obj.numbers[x_238] = temp;
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
  i_1 = (i_1 + 1);
  param_2 = i_1;
  param_3 = h;
  swap_i1_i1_(param_2, param_3);
  int x_276 = i_1;
  return x_276;
}

int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))) ? (1) : (rhs)));
}

void quicksort_() {
  int l_1 = 0;
  int h_1 = 0;
  int top = 0;
  int stack[10] = (int[10])0;
  int int_a = 0;
  int x_278 = 0;
  int x_279 = 0;
  int clamp_a = 0;
  int p = 0;
  int param_4 = 0;
  int param_5 = 0;
  l_1 = 0;
  h_1 = 9;
  top = -1;
  int x_281 = (top + 1);
  top = x_281;
  stack[x_281] = l_1;
  if ((gl_FragCoord.y >= 0.0f)) {
    int x_290 = h_1;
    if (false) {
      x_279 = 1;
    } else {
      x_279 = (h_1 << (0u & 31u));
    }
    x_278 = (x_290 | x_279);
  } else {
    x_278 = 1;
  }
  int_a = x_278;
  int v = int_a;
  clamp_a = min(max(h_1, h_1), v);
  int x_304 = (top + 1);
  top = x_304;
  stack[x_304] = tint_div_i32(clamp_a, 1);
  {
    while(true) {
      if ((top >= 0)) {
      } else {
        break;
      }
      int x_315 = top;
      top = (top - 1);
      h_1 = stack[x_315];
      int x_319 = top;
      top = (top - 1);
      l_1 = stack[x_319];
      param_4 = l_1;
      param_5 = h_1;
      int x_325 = performPartition_i1_i1_(param_4, param_5);
      p = x_325;
      if (((p - 1) > l_1)) {
        int x_333 = (top + 1);
        top = x_333;
        stack[x_333] = l_1;
        int x_337 = (top + 1);
        top = x_337;
        stack[x_337] = (p - 1);
      }
      if (((p + 1) < h_1)) {
        int x_348 = (top + 1);
        top = x_348;
        stack[x_348] = (p + 1);
        int x_353 = (top + 1);
        top = x_353;
        stack[x_353] = h_1;
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
  i_2 = 0;
  {
    while(true) {
      if ((i_2 < 10)) {
      } else {
        break;
      }
      int x_93 = i_2;
      obj.numbers[x_93] = (10 - i_2);
      int x_97 = i_2;
      obj.numbers[x_97] = (obj.numbers[i_2] * obj.numbers[i_2]);
      {
        i_2 = (i_2 + 1);
      }
      continue;
    }
  }
  quicksort_();
  float2 v_1 = gl_FragCoord.xy;
  uv = (v_1 / asfloat(x_34[0u].xy));
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
  float3 x_224 = normalize(color);
  x_GLF_color = float4(x_224[0u], x_224[1u], x_224[2u], 1.0f);
}

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out v_12 = {x_GLF_color};
  return v_12;
}

main_outputs main(main_inputs inputs) {
  main_out v_13 = main_inner(float4(inputs.gl_FragCoord_param.xyz, (1.0f / inputs.gl_FragCoord_param[3u])));
  main_outputs v_14 = {v_13.x_GLF_color_1};
  return v_14;
}

FXC validation failure:
C:\src\dawn\Shader@0x0000025161B977A0(71,11-85): warning X3556: integer divides may be much slower, try using uints if possible.
C:\src\dawn\Shader@0x0000025161B977A0(28,3-20): error X3500: array reference cannot be used as an l-value; not natively addressable
C:\src\dawn\Shader@0x0000025161B977A0(45,5-15): error X3511: forced to unroll loop, but unrolling failed.
C:\src\dawn\Shader@0x0000025161B977A0(110,5-15): error X3511: forced to unroll loop, but unrolling failed.

