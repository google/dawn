SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[19];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int A[17] = int[17](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int r[17] = int[17](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  int a = 0;
  int i = 0;
  bool ok = false;
  int i_1 = 0;
  int x_52 = x_6.x_GLF_uniform_int_values[2].el;
  int x_54 = x_6.x_GLF_uniform_int_values[2].el;
  int x_56 = x_6.x_GLF_uniform_int_values[2].el;
  int x_58 = x_6.x_GLF_uniform_int_values[2].el;
  int x_60 = x_6.x_GLF_uniform_int_values[2].el;
  int x_62 = x_6.x_GLF_uniform_int_values[2].el;
  int x_64 = x_6.x_GLF_uniform_int_values[2].el;
  int x_66 = x_6.x_GLF_uniform_int_values[2].el;
  int x_68 = x_6.x_GLF_uniform_int_values[2].el;
  int x_70 = x_6.x_GLF_uniform_int_values[2].el;
  int x_72 = x_6.x_GLF_uniform_int_values[2].el;
  int x_74 = x_6.x_GLF_uniform_int_values[2].el;
  int x_76 = x_6.x_GLF_uniform_int_values[2].el;
  int x_78 = x_6.x_GLF_uniform_int_values[2].el;
  int x_80 = x_6.x_GLF_uniform_int_values[2].el;
  int x_82 = x_6.x_GLF_uniform_int_values[2].el;
  int x_84 = x_6.x_GLF_uniform_int_values[2].el;
  A = int[17](x_52, x_54, x_56, x_58, x_60, x_62, x_64, x_66, x_68, x_70, x_72, x_74, x_76, x_78, x_80, x_82, x_84);
  int x_87 = x_6.x_GLF_uniform_int_values[3].el;
  int x_89 = x_6.x_GLF_uniform_int_values[4].el;
  int x_91 = x_6.x_GLF_uniform_int_values[5].el;
  int x_93 = x_6.x_GLF_uniform_int_values[6].el;
  int x_95 = x_6.x_GLF_uniform_int_values[7].el;
  int x_97 = x_6.x_GLF_uniform_int_values[8].el;
  int x_99 = x_6.x_GLF_uniform_int_values[9].el;
  int x_101 = x_6.x_GLF_uniform_int_values[10].el;
  int x_103 = x_6.x_GLF_uniform_int_values[11].el;
  int x_105 = x_6.x_GLF_uniform_int_values[12].el;
  int x_107 = x_6.x_GLF_uniform_int_values[13].el;
  int x_109 = x_6.x_GLF_uniform_int_values[14].el;
  int x_111 = x_6.x_GLF_uniform_int_values[15].el;
  int x_113 = x_6.x_GLF_uniform_int_values[16].el;
  int x_115 = x_6.x_GLF_uniform_int_values[17].el;
  int x_117 = x_6.x_GLF_uniform_int_values[18].el;
  int x_119 = x_6.x_GLF_uniform_int_values[1].el;
  r = int[17](x_87, x_89, x_91, x_93, x_95, x_97, x_99, x_101, x_103, x_105, x_107, x_109, x_111, x_113, x_115, x_117, x_119);
  int x_122 = x_6.x_GLF_uniform_int_values[2].el;
  a = x_122;
  int x_124 = x_6.x_GLF_uniform_int_values[2].el;
  i = x_124;
  {
    while(true) {
      int x_129 = i;
      int x_131 = x_6.x_GLF_uniform_int_values[1].el;
      if ((x_129 < x_131)) {
      } else {
        break;
      }
      int x_134 = i;
      int x_135 = a;
      a = (x_135 - 1);
      A[x_134] = x_135;
      int x_138 = i;
      int x_140 = x_6.x_GLF_uniform_int_values[2].el;
      int x_142 = x_6.x_GLF_uniform_int_values[18].el;
      int x_144 = i;
      int x_146 = x_6.x_GLF_uniform_int_values[3].el;
      A[min(max(x_138, x_140), x_142)] = (x_144 + x_146);
      {
        int x_149 = i;
        i = (x_149 + 1);
      }
      continue;
    }
  }
  ok = true;
  int x_152 = x_6.x_GLF_uniform_int_values[2].el;
  i_1 = x_152;
  {
    while(true) {
      int x_157 = i_1;
      int x_159 = x_6.x_GLF_uniform_int_values[1].el;
      if ((x_157 < x_159)) {
      } else {
        break;
      }
      int x_162 = i_1;
      int x_164 = A[x_162];
      int x_165 = i_1;
      int x_167 = r[x_165];
      if ((x_164 != x_167)) {
        ok = false;
      }
      {
        int x_171 = i_1;
        i_1 = (x_171 + 1);
      }
      continue;
    }
  }
  int x_174 = x_6.x_GLF_uniform_int_values[2].el;
  float x_175 = float(x_174);
  x_GLF_color = vec4(x_175, x_175, x_175, x_175);
  bool x_177 = ok;
  if (x_177) {
    int x_181 = x_6.x_GLF_uniform_int_values[3].el;
    int x_184 = x_6.x_GLF_uniform_int_values[2].el;
    int x_187 = x_6.x_GLF_uniform_int_values[2].el;
    int x_190 = x_6.x_GLF_uniform_int_values[3].el;
    float v = float(x_181);
    float v_1 = float(x_184);
    float v_2 = float(x_187);
    x_GLF_color = vec4(v, v_1, v_2, float(x_190));
  }
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
