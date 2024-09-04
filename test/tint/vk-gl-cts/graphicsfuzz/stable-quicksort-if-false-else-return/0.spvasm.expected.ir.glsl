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
void main_1() {
  int x_90 = 0;
  int x_91 = 0;
  int x_92 = 0;
  int x_93 = 0;
  int x_94 = 0;
  int x_95 = 0;
  int x_96 = 0;
  int x_97 = 0;
  int x_98 = 0;
  int x_99 = 0;
  int x_100 = 0;
  int x_101 = 0;
  int x_102 = 0;
  int x_103[10] = int[10](0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int x_104 = 0;
  int x_105 = 0;
  int x_106 = 0;
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
      int x_117 = i_2;
      obj.numbers[x_117] = (10 - i_2);
      int x_121 = i_2;
      obj.numbers[x_121] = (obj.numbers[i_2] * obj.numbers[i_2]);
      {
        i_2 = (i_2 + 1);
      }
      continue;
    }
  }
  x_100 = 0;
  x_101 = 9;
  x_102 = -1;
  int x_133 = (x_102 + 1);
  x_102 = x_133;
  x_103[x_133] = x_100;
  int x_137 = (x_102 + 1);
  x_102 = x_137;
  x_103[x_137] = x_101;
  {
    while(true) {
      if ((x_102 >= 0)) {
      } else {
        break;
      }
      int x_147 = x_102;
      x_102 = (x_102 - 1);
      x_101 = x_103[x_147];
      int x_151 = x_102;
      x_102 = (x_102 - 1);
      x_100 = x_103[x_151];
      x_105 = x_100;
      x_106 = x_101;
      x_92 = obj.numbers[x_106];
      x_93 = (x_105 - 1);
      x_94 = x_105;
      {
        while(true) {
          if ((x_94 <= (x_106 - 1))) {
          } else {
            break;
          }
          if ((obj.numbers[x_94] <= x_92)) {
            x_93 = (x_93 + 1);
            x_95 = x_93;
            x_96 = x_94;
            x_91 = obj.numbers[x_95];
            int x_186 = x_95;
            obj.numbers[x_186] = obj.numbers[x_96];
            int x_191 = x_96;
            obj.numbers[x_191] = x_91;
          }
          {
            x_94 = (x_94 + 1);
          }
          continue;
        }
      }
      x_97 = (x_93 + 1);
      x_98 = x_106;
      x_90 = obj.numbers[x_97];
      int x_202 = x_97;
      obj.numbers[x_202] = obj.numbers[x_98];
      int x_207 = x_98;
      obj.numbers[x_207] = x_90;
      x_99 = (x_93 + 1);
      x_104 = x_99;
      if (((x_104 - 1) > x_100)) {
        int x_220 = (x_102 + 1);
        x_102 = x_220;
        x_103[x_220] = x_100;
        int x_224 = (x_102 + 1);
        x_102 = x_224;
        x_103[x_224] = (x_104 - 1);
      }
      if (((x_104 + 1) < x_101)) {
        int x_235 = (x_102 + 1);
        x_102 = x_235;
        x_103[x_235] = (x_104 + 1);
        int x_240 = (x_102 + 1);
        x_102 = x_240;
        x_103[x_240] = x_101;
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
  vec3 x_358 = normalize(color);
  frag_color = vec4(x_358[0u], x_358[1u], x_358[2u], 1.0f);
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
  int x_369 = i;
  obj.numbers[x_369] = obj.numbers[j];
  int x_374 = j;
  obj.numbers[x_374] = temp;
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
  int x_411 = i_1;
  return (x_411 + 1);
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
  int x_415 = (top + 1);
  top = x_415;
  stack[x_415] = l_1;
  int x_419 = (top + 1);
  top = x_419;
  stack[x_419] = h_1;
  {
    while(true) {
      if ((top >= 0)) {
      } else {
        break;
      }
      int x_429 = top;
      top = (top - 1);
      h_1 = stack[x_429];
      int x_433 = top;
      top = (top - 1);
      l_1 = stack[x_433];
      param_4 = l_1;
      param_5 = h_1;
      int x_439 = performPartition_i1_i1_(param_4, param_5);
      p = x_439;
      if (((p - 1) > l_1)) {
        int x_447 = (top + 1);
        top = x_447;
        stack[x_447] = l_1;
        int x_451 = (top + 1);
        top = x_451;
        stack[x_451] = (p - 1);
      }
      if (((p + 1) < h_1)) {
        int x_462 = (top + 1);
        top = x_462;
        stack[x_462] = (p + 1);
        int x_467 = (top + 1);
        top = x_467;
        stack[x_467] = h_1;
      }
      {
      }
      continue;
    }
  }
}
error: Error parsing GLSL shader:
ERROR: 0:179: 'main' : function cannot take any parameter(s) 
ERROR: 0:179: 'structure' :  entry point cannot return a value
ERROR: 0:179: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
