SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[16];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int r[15] = int[15](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int i = 0;
  int data[15] = int[15](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int i_1 = 0;
  int i_2 = 0;
  int i_3 = 0;
  int x_46 = x_6.x_GLF_uniform_int_values[0].el;
  int x_48 = x_6.x_GLF_uniform_int_values[0].el;
  r[x_46] = x_48;
  int x_51 = x_6.x_GLF_uniform_int_values[1].el;
  int x_53 = x_6.x_GLF_uniform_int_values[1].el;
  r[x_51] = x_53;
  int x_56 = x_6.x_GLF_uniform_int_values[2].el;
  int x_58 = x_6.x_GLF_uniform_int_values[2].el;
  r[x_56] = x_58;
  int x_61 = x_6.x_GLF_uniform_int_values[3].el;
  int x_63 = x_6.x_GLF_uniform_int_values[3].el;
  r[x_61] = x_63;
  int x_66 = x_6.x_GLF_uniform_int_values[4].el;
  int x_68 = x_6.x_GLF_uniform_int_values[4].el;
  r[x_66] = x_68;
  int x_71 = x_6.x_GLF_uniform_int_values[5].el;
  int x_73 = x_6.x_GLF_uniform_int_values[1].el;
  r[x_71] = -(x_73);
  int x_77 = x_6.x_GLF_uniform_int_values[8].el;
  int x_79 = x_6.x_GLF_uniform_int_values[1].el;
  r[x_77] = -(x_79);
  int x_83 = x_6.x_GLF_uniform_int_values[9].el;
  int x_85 = x_6.x_GLF_uniform_int_values[1].el;
  r[x_83] = -(x_85);
  int x_89 = x_6.x_GLF_uniform_int_values[10].el;
  int x_91 = x_6.x_GLF_uniform_int_values[1].el;
  r[x_89] = -(x_91);
  int x_95 = x_6.x_GLF_uniform_int_values[11].el;
  int x_97 = x_6.x_GLF_uniform_int_values[1].el;
  r[x_95] = -(x_97);
  int x_101 = x_6.x_GLF_uniform_int_values[6].el;
  int x_103 = x_6.x_GLF_uniform_int_values[2].el;
  r[x_101] = -(x_103);
  int x_107 = x_6.x_GLF_uniform_int_values[12].el;
  int x_109 = x_6.x_GLF_uniform_int_values[2].el;
  r[x_107] = -(x_109);
  int x_113 = x_6.x_GLF_uniform_int_values[13].el;
  int x_115 = x_6.x_GLF_uniform_int_values[2].el;
  r[x_113] = -(x_115);
  int x_119 = x_6.x_GLF_uniform_int_values[14].el;
  int x_121 = x_6.x_GLF_uniform_int_values[2].el;
  r[x_119] = -(x_121);
  int x_125 = x_6.x_GLF_uniform_int_values[15].el;
  int x_127 = x_6.x_GLF_uniform_int_values[2].el;
  r[x_125] = -(x_127);
  i = 0;
  {
    while(true) {
      int x_134 = i;
      int x_136 = x_6.x_GLF_uniform_int_values[5].el;
      if ((x_134 < x_136)) {
      } else {
        break;
      }
      int x_139 = i;
      int x_140 = i;
      int x_142 = i;
      int x_145 = x_6.x_GLF_uniform_int_values[1].el;
      data[x_139] = ~(min(max(~(x_140), ~(x_142)), x_145));
      {
        int x_149 = i;
        i = (x_149 + 1);
      }
      continue;
    }
  }
  int x_152 = x_6.x_GLF_uniform_int_values[5].el;
  i_1 = x_152;
  {
    while(true) {
      int x_157 = i_1;
      int x_159 = x_6.x_GLF_uniform_int_values[6].el;
      if ((x_157 < x_159)) {
      } else {
        break;
      }
      int x_162 = i_1;
      int x_163 = i_1;
      data[x_162] = ~(min(max(~(x_163), 0), 1));
      {
        int x_168 = i_1;
        i_1 = (x_168 + 1);
      }
      continue;
    }
  }
  int x_171 = x_6.x_GLF_uniform_int_values[6].el;
  i_2 = x_171;
  {
    while(true) {
      int x_176 = i_2;
      int x_178 = x_6.x_GLF_uniform_int_values[7].el;
      if ((x_176 < x_178)) {
      } else {
        break;
      }
      int x_181 = i_2;
      int x_182 = i_2;
      data[x_181] = ~(min(max(x_182, 0), 1));
      {
        int x_186 = i_2;
        i_2 = (x_186 + 1);
      }
      continue;
    }
  }
  int x_189 = x_6.x_GLF_uniform_int_values[0].el;
  i_3 = x_189;
  {
    while(true) {
      int x_194 = i_3;
      int x_196 = x_6.x_GLF_uniform_int_values[7].el;
      if ((x_194 < x_196)) {
      } else {
        break;
      }
      int x_199 = i_3;
      int x_201 = data[x_199];
      int x_202 = i_3;
      int x_204 = r[x_202];
      if ((x_201 != x_204)) {
        int x_209 = x_6.x_GLF_uniform_int_values[0].el;
        float x_210 = float(x_209);
        x_GLF_color = vec4(x_210, x_210, x_210, x_210);
        return;
      }
      {
        int x_212 = i_3;
        i_3 = (x_212 + 1);
      }
      continue;
    }
  }
  int x_215 = x_6.x_GLF_uniform_int_values[1].el;
  int x_218 = x_6.x_GLF_uniform_int_values[0].el;
  int x_221 = x_6.x_GLF_uniform_int_values[0].el;
  int x_224 = x_6.x_GLF_uniform_int_values[1].el;
  float v = float(x_215);
  float v_1 = float(x_218);
  float v_2 = float(x_221);
  x_GLF_color = vec4(v, v_1, v_2, float(x_224));
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
