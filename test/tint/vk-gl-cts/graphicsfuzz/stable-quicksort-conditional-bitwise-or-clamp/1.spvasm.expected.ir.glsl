SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct QuicksortObject {
  int numbers[10];
};

struct buf0 {
  vec2 resolution;
};

struct main_out {
  vec4 x_GLF_color_1;
};

QuicksortObject obj = QuicksortObject(int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
vec4 tint_symbol = vec4(0.0f);
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
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
  return (lhs / ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs)));
}
void quicksort_() {
  int l_1 = 0;
  int h_1 = 0;
  int top = 0;
  int stack[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
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
  if ((tint_symbol.y >= 0.0f)) {
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
  int v_1 = int_a;
  clamp_a = min(max(h_1, h_1), v_1);
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
  vec2 uv = vec2(0.0f);
  vec3 color = vec3(0.0f);
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
  uv = (tint_symbol.xy / v.tint_symbol_3.resolution);
  color = vec3(1.0f, 2.0f, 3.0f);
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
  vec3 x_224 = normalize(color);
  x_GLF_color = vec4(x_224[0u], x_224[1u], x_224[2u], 1.0f);
}
main_out tint_symbol_1_inner(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_1_loc0_Output = tint_symbol_1_inner(gl_FragCoord).x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:71: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:71: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:71: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
