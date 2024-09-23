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
  bvec4 x_116 = bvec4(false);
  if ((pos.x < quad.x)) {
    return false;
  }
  if ((pos.y < quad.y)) {
    return false;
  }
  if ((pos.x > (quad.x + quad.z))) {
    return false;
  }
  if ((pos.y > (quad.y + quad.w))) {
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
      if ((i < 8)) {
      } else {
        break;
      }
      int x_159 = i;
      param = pos_1;
      indexable = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
      param_1 = indexable[x_159];
      bool x_163 = collision_vf2_vf4_(param, param_1);
      if (x_163) {
        int x_166 = i;
        indexable_1 = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
        float x_168 = indexable_1[x_166].x;
        int x_170 = i;
        indexable_2 = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
        float x_172 = indexable_2[x_170].y;
        int x_175 = i;
        indexable_3 = vec4[16](vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.5f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.5f, 0.0f, 1.0f), vec4(0.5f, 0.5f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 0.5f, 1.0f), vec4(0.5f, 0.0f, 0.5f, 1.0f), vec4(0.0f, 0.5f, 0.5f, 1.0f), vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f));
        int v_2 = tint_f32_to_i32(x_168);
        res = indexable_3[tint_mod_i32((((v_2 * tint_f32_to_i32(x_172)) + (x_175 * 9)) + 11), 16)];
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  vec4 x_184 = res;
  return x_184;
}
void main_1() {
  vec2 lin = vec2(0.0f);
  vec2 param_2 = vec2(0.0f);
  lin = (tint_symbol.xy / v.tint_symbol_3.resolution);
  lin = floor((lin * 32.0f));
  param_2 = lin;
  vec4 x_114 = match_vf2_(param_2);
  x_GLF_color = x_114;
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
ERROR: 0:38: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:38: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:38: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
