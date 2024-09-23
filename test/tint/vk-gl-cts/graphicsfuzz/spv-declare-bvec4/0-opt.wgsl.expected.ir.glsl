SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  vec2 resolution;
};

struct main_out {
  vec4 x_GLF_color_1;
};

vec4 tint_symbol = vec4(0.0f);
layout(binding = 0, std140)
uniform tint_symbol_4_1_ubo {
  buf0 tint_symbol_3;
} v;
vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
bool collision_vf2_vf4_(inout vec2 pos, inout vec4 quad) {
  float x_114 = pos.x;
  float x_116 = quad.x;
  if ((x_114 < x_116)) {
    return false;
  }
  float x_121 = pos.y;
  float x_123 = quad.y;
  if ((x_121 < x_123)) {
    return false;
  }
  float x_128 = pos.x;
  float x_130 = quad.x;
  float x_132 = quad.z;
  if ((x_128 > (x_130 + x_132))) {
    return false;
  }
  float x_138 = pos.y;
  float x_140 = quad.y;
  float x_142 = quad.w;
  if ((x_138 > (x_140 + x_142))) {
    return false;
  }
  return true;
}
int tint_mod_i32(int lhs, int rhs) {
  int v_1 = ((((rhs == 0) | ((lhs == (-2147483647 - 1)) & (rhs == -1)))) ? (1) : (rhs));
  return (lhs - ((lhs / v_1) * v_1));
}
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : ((-2147483647 - 1)))) : (2147483647));
}
vec4 match_vf2_(inout vec2 pos_1) {
  vec4 res = vec4(0.0f);
  int i = 0;
  vec2 param = vec2(0.0f);
  vec4 param_1 = vec4(0.0f);
  vec4 indexable[8] = vec4[8](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 indexable_1[8] = vec4[8](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 indexable_2[8] = vec4[8](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 indexable_3[16] = vec4[16](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  res = vec4(0.5f, 0.5f, 1.0f, 1.0f);
  i = 0;
  {
    while(true) {
      int x_152 = i;
      if ((x_152 < 8)) {
      } else {
        break;
      }
      int x_155 = i;
      vec2 x_156 = pos_1;
      param = x_156;
      indexable = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
      vec4 x_158 = indexable[x_155];
      param_1 = x_158;
      bool x_159 = collision_vf2_vf4_(param, param_1);
      if (x_159) {
        int x_162 = i;
        indexable_1 = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
        float x_164 = indexable_1[x_162].x;
        int x_166 = i;
        indexable_2 = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
        float x_168 = indexable_2[x_166].y;
        int x_171 = i;
        indexable_3 = vec4[16](vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.5f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.5f, 0.0f, 1.0f), vec4(0.5f, 0.5f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 0.5f, 1.0f), vec4(0.5f, 0.0f, 0.5f, 1.0f), vec4(0.0f, 0.5f, 0.5f, 1.0f), vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f));
        int v_2 = tint_f32_to_i32(x_164);
        vec4 x_177 = indexable_3[tint_mod_i32((((v_2 * tint_f32_to_i32(x_168)) + (x_171 * 9)) + 11), 16)];
        res = x_177;
      }
      {
        int x_178 = i;
        i = (x_178 + 1);
      }
      continue;
    }
  }
  vec4 x_180 = res;
  return x_180;
}
void main_1() {
  vec2 lin = vec2(0.0f);
  vec2 param_2 = vec2(0.0f);
  vec4 x_102 = tint_symbol;
  vec2 x_105 = v.tint_symbol_3.resolution;
  lin = (vec2(x_102[0u], x_102[1u]) / x_105);
  vec2 x_107 = lin;
  lin = floor((x_107 * 32.0f));
  vec2 x_110 = lin;
  param_2 = x_110;
  vec4 x_111 = match_vf2_(param_2);
  x_GLF_color = x_111;
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
ERROR: 0:47: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:47: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:47: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
