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
vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_30;
void swap_i1_i1_(inout int i, inout int j) {
  int temp = 0;
  temp = obj.numbers[i];
  int x_95 = i;
  obj.numbers[x_95] = obj.numbers[j];
  int x_100 = j;
  obj.numbers[x_100] = temp;
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
  int x_137 = i_1;
  return (x_137 + 1);
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
  int x_141 = (top + 1);
  top = x_141;
  stack[x_141] = l_1;
  int x_145 = (top + 1);
  top = x_145;
  stack[x_145] = h_1;
  {
    while(true) {
      if ((top >= 0)) {
      } else {
        break;
      }
      int x_155 = top;
      top = (top - 1);
      h_1 = stack[x_155];
      int x_159 = top;
      top = (top - 1);
      l_1 = stack[x_159];
      param_4 = l_1;
      param_5 = h_1;
      int x_165 = performPartition_i1_i1_(param_4, param_5);
      p = x_165;
      if (((p - 1) > l_1)) {
        int x_173 = (top + 1);
        top = x_173;
        stack[x_173] = l_1;
        int x_177 = (top + 1);
        top = x_177;
        stack[x_177] = (p - 1);
      }
      if (((p + 1) < h_1)) {
        int x_188 = (top + 1);
        top = x_188;
        stack[x_188] = (p + 1);
        int x_193 = (top + 1);
        top = x_193;
        stack[x_193] = h_1;
      }
      {
      }
      continue;
    }
  }
}
void main_1() {
  int i_2 = 0;
  i_2 = 0;
  {
    while(true) {
      if ((i_2 < 10)) {
      } else {
        break;
      }
      int x_67 = i_2;
      obj.numbers[x_67] = (10 - i_2);
      int x_71 = i_2;
      obj.numbers[x_71] = (obj.numbers[i_2] * obj.numbers[i_2]);
      {
        i_2 = (i_2 + 1);
      }
      continue;
    }
  }
  quicksort_();
  if ((obj.numbers[0] < obj.numbers[4])) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
