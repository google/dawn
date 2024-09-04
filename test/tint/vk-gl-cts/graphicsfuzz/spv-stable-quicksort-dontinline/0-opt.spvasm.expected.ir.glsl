SKIP: FAILED

#version 310 es

struct QuicksortObject {
  int numbers[10];
};

struct buf0 {
  vec2 resolution;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


QuicksortObject obj = QuicksortObject(int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_32;
vec4 x_GLF_color = vec4(0.0f);
void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  temp = obj.numbers[i];
  int x_228 = i;
  obj.numbers[x_228] = obj.numbers[j];
  int x_233 = j;
  obj.numbers[x_233] = temp;
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
  int x_271 = i_1;
  return x_271;
}
void quicksort_() {
  int l_1 = 0;
  int h_1 = 0;
  int top = 0;
  int stack[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int p = 0;
  int param_4 = 0;
  int param_5 = 0;
  l_1 = 0;
  h_1 = 9;
  top = -1;
  int x_274 = (top + 1);
  top = x_274;
  stack[x_274] = l_1;
  int x_278 = (top + 1);
  top = x_278;
  stack[x_278] = h_1;
  {
    while(true) {
      if ((top >= 0)) {
      } else {
        break;
      }
      int x_288 = top;
      top = (top - 1);
      h_1 = stack[x_288];
      int x_292 = top;
      top = (top - 1);
      l_1 = stack[x_292];
      param_4 = l_1;
      param_5 = h_1;
      int x_298 = performPartition_i1_i1_(param_4, param_5);
      p = x_298;
      if (((p - 1) > l_1)) {
        int x_306 = (top + 1);
        top = x_306;
        stack[x_306] = l_1;
        int x_310 = (top + 1);
        top = x_310;
        stack[x_310] = (p - 1);
      }
      if (((p + 1) < h_1)) {
        int x_321 = (top + 1);
        top = x_321;
        stack[x_321] = (p + 1);
        int x_326 = (top + 1);
        top = x_326;
        stack[x_326] = h_1;
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
      int x_88 = i_2;
      obj.numbers[x_88] = (10 - i_2);
      int x_92 = i_2;
      obj.numbers[x_92] = (obj.numbers[i_2] * obj.numbers[i_2]);
      {
        i_2 = (i_2 + 1);
      }
      continue;
    }
  }
  quicksort_();
  uv = (tint_symbol.xy / x_32.resolution);
  color = vec3(1.0f, 2.0f, 3.0f);
  float v = color.x;
  color[0u] = (v + float(obj.numbers[0]));
  if ((uv.x > 0.25f)) {
    float v_1 = color.x;
    color[0u] = (v_1 + float(obj.numbers[1]));
  }
  if ((uv.x > 0.5f)) {
    float v_2 = color.y;
    color[1u] = (v_2 + float(obj.numbers[2]));
  }
  if ((uv.x > 0.75f)) {
    float v_3 = color.z;
    color[2u] = (v_3 + float(obj.numbers[3]));
  }
  float v_4 = color.y;
  color[1u] = (v_4 + float(obj.numbers[4]));
  if ((uv.y > 0.25f)) {
    float v_5 = color.x;
    color[0u] = (v_5 + float(obj.numbers[5]));
  }
  if ((uv.y > 0.5f)) {
    float v_6 = color.y;
    color[1u] = (v_6 + float(obj.numbers[6]));
  }
  if ((uv.y > 0.75f)) {
    float v_7 = color.z;
    color[2u] = (v_7 + float(obj.numbers[7]));
  }
  float v_8 = color.z;
  color[2u] = (v_8 + float(obj.numbers[8]));
  if ((abs((uv.x - uv.y)) < 0.25f)) {
    float v_9 = color.x;
    color[0u] = (v_9 + float(obj.numbers[9]));
  }
  vec3 x_219 = normalize(color);
  x_GLF_color = vec4(x_219[0u], x_219[1u], x_219[2u], 1.0f);
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
