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
int performPartition_i1_i1_(inout int l, inout int h) {
  int x_314 = 0;
  int x_315 = 0;
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
        x_315 = obj.numbers[param];
        int x_345 = param;
        obj.numbers[x_345] = obj.numbers[param_1];
        int x_350 = param_1;
        obj.numbers[x_350] = x_315;
      }
      {
        j_1 = (j_1 + 1);
      }
      continue;
    }
  }
  param_2 = (i_1 + 1);
  param_3 = h;
  x_314 = obj.numbers[param_2];
  int x_361 = param_2;
  obj.numbers[x_361] = obj.numbers[param_3];
  int x_366 = param_3;
  obj.numbers[x_366] = x_314;
  if (false) {
  } else {
    int x_372 = i_1;
    return (x_372 + 1);
  }
  return 0;
}
void main_1() {
  int x_91 = 0;
  int x_92 = 0;
  int x_93 = 0;
  int x_94[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int x_95 = 0;
  int x_96 = 0;
  int x_97 = 0;
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
      int x_108 = i_2;
      obj.numbers[x_108] = (10 - i_2);
      int x_112 = i_2;
      obj.numbers[x_112] = (obj.numbers[i_2] * obj.numbers[i_2]);
      {
        i_2 = (i_2 + 1);
      }
      continue;
    }
  }
  x_91 = 0;
  x_92 = 9;
  x_93 = -1;
  int x_124 = (x_93 + 1);
  x_93 = x_124;
  x_94[x_124] = x_91;
  int x_128 = (x_93 + 1);
  x_93 = x_128;
  x_94[x_128] = x_92;
  {
    while(true) {
      if ((x_93 >= 0)) {
      } else {
        break;
      }
      int x_138 = x_93;
      x_93 = (x_93 - 1);
      x_92 = x_94[x_138];
      int x_142 = x_93;
      x_93 = (x_93 - 1);
      x_91 = x_94[x_142];
      x_96 = x_91;
      x_97 = x_92;
      int x_148 = performPartition_i1_i1_(x_96, x_97);
      x_95 = x_148;
      if (((x_95 - 1) > x_91)) {
        int x_156 = (x_93 + 1);
        x_93 = x_156;
        x_94[x_156] = x_91;
        int x_160 = (x_93 + 1);
        x_93 = x_160;
        x_94[x_160] = (x_95 - 1);
      }
      if (((x_95 + 1) < x_92)) {
        int x_171 = (x_93 + 1);
        x_93 = x_171;
        x_94[x_171] = (x_95 + 1);
        int x_176 = (x_93 + 1);
        x_93 = x_176;
        x_94[x_176] = x_92;
      }
      {
      }
      continue;
    }
  }
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
  vec3 x_294 = normalize(color);
  frag_color = vec4(x_294[0u], x_294[1u], x_294[2u], 1.0f);
  tint_symbol = x_GLF_pos;
}
main_out main(vec4 x_GLF_pos_param) {
  x_GLF_pos = x_GLF_pos_param;
  main_1();
  return main_out(frag_color, tint_symbol);
}
void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  temp = obj.numbers[i];
  int x_305 = i;
  obj.numbers[x_305] = obj.numbers[j];
  int x_310 = j;
  obj.numbers[x_310] = temp;
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
  int x_377 = (top + 1);
  top = x_377;
  stack[x_377] = l_1;
  int x_381 = (top + 1);
  top = x_381;
  stack[x_381] = h_1;
  {
    while(true) {
      if ((top >= 0)) {
      } else {
        break;
      }
      int x_391 = top;
      top = (top - 1);
      h_1 = stack[x_391];
      int x_395 = top;
      top = (top - 1);
      l_1 = stack[x_395];
      param_4 = l_1;
      param_5 = h_1;
      int x_401 = performPartition_i1_i1_(param_4, param_5);
      p = x_401;
      if (((p - 1) > l_1)) {
        int x_409 = (top + 1);
        top = x_409;
        stack[x_409] = l_1;
        int x_413 = (top + 1);
        top = x_413;
        stack[x_413] = (p - 1);
      }
      if (((p + 1) < h_1)) {
        int x_424 = (top + 1);
        top = x_424;
        stack[x_424] = (p + 1);
        int x_429 = (top + 1);
        top = x_429;
        stack[x_429] = h_1;
      }
      {
      }
      continue;
    }
  }
}
error: Error parsing GLSL shader:
ERROR: 0:186: 'main' : function cannot take any parameter(s) 
ERROR: 0:186: 'structure' :  entry point cannot return a value
ERROR: 0:186: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
