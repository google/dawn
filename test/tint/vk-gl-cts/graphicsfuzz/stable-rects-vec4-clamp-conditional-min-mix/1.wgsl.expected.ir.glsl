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
  float x_110 = pos.x;
  float x_112 = quad.x;
  if ((x_110 < x_112)) {
    return false;
  }
  float x_117 = pos.y;
  float x_119 = quad.y;
  if ((x_117 < x_119)) {
    return false;
  }
  float x_124 = pos.x;
  float x_126 = quad.x;
  float x_128 = quad.z;
  if ((x_124 > (x_126 + x_128))) {
    return false;
  }
  float x_134 = pos.y;
  float x_136 = quad.y;
  float x_138 = quad.w;
  if ((x_134 > (x_136 + x_138))) {
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
  float x_144 = 0.0f;
  float x_145 = 0.0f;
  int i = 0;
  vec2 param = vec2(0.0f);
  vec4 param_1 = vec4(0.0f);
  vec4 indexable[8] = vec4[8](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 indexable_1[8] = vec4[8](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 indexable_2[8] = vec4[8](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 indexable_3[16] = vec4[16](vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  float x_147 = tint_symbol.x;
  if ((x_147 < 0.0f)) {
    x_144 = -1.0f;
  } else {
    float x_153 = tint_symbol.x;
    if ((x_153 >= 0.0f)) {
      float x_159 = tint_symbol.x;
      x_145 = (((x_159 >= 0.0f)) ? (0.5f) : (1.0f));
    } else {
      x_145 = 1.0f;
    }
    float x_162 = x_145;
    x_144 = min(x_162, 0.5f);
  }
  float x_164 = x_144;
  res = vec4(clamp(0.5f, 0.5f, x_164), 0.5f, 1.0f, 1.0f);
  i = 0;
  {
    while(true) {
      int x_171 = i;
      if ((x_171 < 8)) {
      } else {
        break;
      }
      int x_174 = i;
      vec2 x_175 = pos_1;
      param = x_175;
      indexable = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
      vec4 x_177 = indexable[x_174];
      param_1 = x_177;
      bool x_178 = collision_vf2_vf4_(param, param_1);
      if (x_178) {
        int x_181 = i;
        indexable_1 = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
        float x_183 = indexable_1[x_181].x;
        int x_185 = i;
        indexable_2 = vec4[8](vec4(4.0f, 4.0f, 20.0f, 4.0f), vec4(4.0f, 4.0f, 4.0f, 20.0f), vec4(4.0f, 20.0f, 20.0f, 4.0f), vec4(20.0f, 4.0f, 4.0f, 8.0f), vec4(8.0f, 6.0f, 4.0f, 2.0f), vec4(2.0f, 12.0f, 2.0f, 4.0f), vec4(16.0f, 2.0f, 4.0f, 4.0f), vec4(12.0f, 22.0f, 4.0f, 4.0f));
        float x_187 = indexable_2[x_185].y;
        int x_190 = i;
        indexable_3 = vec4[16](vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.5f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.5f, 0.0f, 1.0f), vec4(0.5f, 0.5f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 0.5f, 1.0f), vec4(0.5f, 0.0f, 0.5f, 1.0f), vec4(0.0f, 0.5f, 0.5f, 1.0f), vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f), vec4(1.0f));
        int v_2 = tint_f32_to_i32(x_183);
        vec4 x_196 = indexable_3[tint_mod_i32((((v_2 * tint_f32_to_i32(x_187)) + (x_190 * 9)) + 11), 16)];
        res = x_196;
      }
      {
        int x_197 = i;
        i = (x_197 + 1);
      }
      continue;
    }
  }
  vec4 x_199 = res;
  return x_199;
}
void main_1() {
  vec2 lin = vec2(0.0f);
  vec2 param_2 = vec2(0.0f);
  vec4 x_98 = tint_symbol;
  vec2 x_101 = v.tint_symbol_3.resolution;
  lin = (vec2(x_98[0u], x_98[1u]) / x_101);
  vec2 x_103 = lin;
  lin = floor((x_103 * 32.0f));
  vec2 x_106 = lin;
  param_2 = x_106;
  vec4 x_107 = match_vf2_(param_2);
  x_GLF_color = x_107;
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
