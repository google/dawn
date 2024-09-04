SKIP: FAILED

#version 310 es

struct QuicksortObject {
  int numbers[10];
};

struct buf0 {
  vec2 resolution;
};

struct main_out {
  vec4 frag_color_1;
  vec4 tint_symbol;
};

QuicksortObject obj = QuicksortObject(int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
vec4 x_GLF_FragCoord = vec4(0.0f);
vec4 x_GLF_pos = vec4(0.0f);
uniform buf0 x_34;
vec4 frag_color = vec4(0.0f);
vec4 tint_symbol = vec4(0.0f);
void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  temp = obj.numbers[i];
  int x_242 = i;
  obj.numbers[x_242] = obj.numbers[j];
  int x_247 = j;
  obj.numbers[x_247] = temp;
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
  int x_284 = i_1;
  return (x_284 + 1);
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
  int x_288 = (top + 1);
  top = x_288;
  stack[x_288] = l_1;
  int x_292 = (top + 1);
  top = x_292;
  stack[x_292] = h_1;
  {
    while(true) {
      if ((top >= 0)) {
      } else {
        break;
      }
      int x_302 = top;
      top = (top - 1);
      h_1 = stack[x_302];
      int x_306 = top;
      top = (top - 1);
      l_1 = stack[x_306];
      param_4 = l_1;
      param_5 = h_1;
      int x_312 = performPartition_i1_i1_(param_4, param_5);
      p = x_312;
      if (((p - 1) > l_1)) {
        int x_320 = (top + 1);
        top = x_320;
        stack[x_320] = l_1;
        int x_324 = (top + 1);
        top = x_324;
        stack[x_324] = (p - 1);
      }
      if (((p + 1) < h_1)) {
        int x_335 = (top + 1);
        top = x_335;
        stack[x_335] = (p + 1);
        int x_340 = (top + 1);
        top = x_340;
        stack[x_340] = h_1;
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
  x_GLF_FragCoord = ((x_GLF_pos + vec4(1.0f, 1.0f, 0.0f, 0.0f)) * vec4(128.0f, 128.0f, 1.0f, 1.0f));
  i_2 = 0;
  {
    while(true) {
      if ((i_2 < 10)) {
      } else {
        break;
      }
      int x_100 = i_2;
      obj.numbers[x_100] = (10 - i_2);
      int x_104 = i_2;
      obj.numbers[x_104] = (obj.numbers[i_2] * obj.numbers[i_2]);
      {
        i_2 = (i_2 + 1);
      }
      continue;
    }
  }
  quicksort_();
  uv = (x_GLF_FragCoord.xy / x_34.resolution);
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
  vec3 x_231 = normalize(color);
  frag_color = vec4(x_231[0u], x_231[1u], x_231[2u], 1.0f);
  tint_symbol = x_GLF_pos;
}
main_out main(vec4 x_GLF_pos_param) {
  x_GLF_pos = x_GLF_pos_param;
  main_1();
  return main_out(frag_color, tint_symbol);
}
error: Error parsing GLSL shader:
ERROR: 0:183: 'main' : function cannot take any parameter(s) 
ERROR: 0:183: 'structure' :  entry point cannot return a value
ERROR: 0:183: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
