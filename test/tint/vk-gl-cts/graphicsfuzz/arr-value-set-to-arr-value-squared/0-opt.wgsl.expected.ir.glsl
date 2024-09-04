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
  int x_92 = i;
  int x_94 = obj.numbers[x_92];
  temp = x_94;
  int x_95 = i;
  int x_96 = j;
  int x_98 = obj.numbers[x_96];
  obj.numbers[x_95] = x_98;
  int x_100 = j;
  int x_101 = temp;
  obj.numbers[x_100] = x_101;
}
int performPartition_i1_i1_(inout int l, inout int h) {
  int pivot = 0;
  int i_1 = 0;
  int j_1 = 0;
  int param = 0;
  int param_1 = 0;
  int param_2 = 0;
  int param_3 = 0;
  int x_104 = h;
  int x_106 = obj.numbers[x_104];
  pivot = x_106;
  int x_107 = l;
  i_1 = (x_107 - 1);
  int x_109 = l;
  j_1 = x_109;
  {
    while(true) {
      int x_114 = j_1;
      int x_115 = h;
      if ((x_114 <= (x_115 - 1))) {
      } else {
        break;
      }
      int x_119 = j_1;
      int x_121 = obj.numbers[x_119];
      int x_122 = pivot;
      if ((x_121 <= x_122)) {
        int x_126 = i_1;
        i_1 = (x_126 + 1);
        int x_128 = i_1;
        param = x_128;
        int x_129 = j_1;
        param_1 = x_129;
        swap_i1_i1_(param, param_1);
      }
      {
        int x_131 = j_1;
        j_1 = (x_131 + 1);
      }
      continue;
    }
  }
  int x_133 = i_1;
  param_2 = (x_133 + 1);
  int x_135 = h;
  param_3 = x_135;
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
  int x_140 = top;
  int x_141 = (x_140 + 1);
  top = x_141;
  int x_142 = l_1;
  stack[x_141] = x_142;
  int x_144 = top;
  int x_145 = (x_144 + 1);
  top = x_145;
  int x_146 = h_1;
  stack[x_145] = x_146;
  {
    while(true) {
      int x_152 = top;
      if ((x_152 >= 0)) {
      } else {
        break;
      }
      int x_155 = top;
      top = (x_155 - 1);
      int x_158 = stack[x_155];
      h_1 = x_158;
      int x_159 = top;
      top = (x_159 - 1);
      int x_162 = stack[x_159];
      l_1 = x_162;
      int x_163 = l_1;
      param_4 = x_163;
      int x_164 = h_1;
      param_5 = x_164;
      int x_165 = performPartition_i1_i1_(param_4, param_5);
      p = x_165;
      int x_166 = p;
      int x_168 = l_1;
      if (((x_166 - 1) > x_168)) {
        int x_172 = top;
        int x_173 = (x_172 + 1);
        top = x_173;
        int x_174 = l_1;
        stack[x_173] = x_174;
        int x_176 = top;
        int x_177 = (x_176 + 1);
        top = x_177;
        int x_178 = p;
        stack[x_177] = (x_178 - 1);
      }
      int x_181 = p;
      int x_183 = h_1;
      if (((x_181 + 1) < x_183)) {
        int x_187 = top;
        int x_188 = (x_187 + 1);
        top = x_188;
        int x_189 = p;
        stack[x_188] = (x_189 + 1);
        int x_192 = top;
        int x_193 = (x_192 + 1);
        top = x_193;
        int x_194 = h_1;
        stack[x_193] = x_194;
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
      int x_64 = i_2;
      if ((x_64 < 10)) {
      } else {
        break;
      }
      int x_67 = i_2;
      int x_68 = i_2;
      obj.numbers[x_67] = (10 - x_68);
      int x_71 = i_2;
      int x_72 = i_2;
      int x_74 = obj.numbers[x_72];
      int x_75 = i_2;
      int x_77 = obj.numbers[x_75];
      obj.numbers[x_71] = (x_74 * x_77);
      {
        int x_80 = i_2;
        i_2 = (x_80 + 1);
      }
      continue;
    }
  }
  quicksort_();
  int x_84 = obj.numbers[0];
  int x_86 = obj.numbers[4];
  if ((x_84 < x_86)) {
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
