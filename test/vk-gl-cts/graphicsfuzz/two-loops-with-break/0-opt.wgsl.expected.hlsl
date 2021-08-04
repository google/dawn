void set_float4(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 GLF_live15c = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int GLF_live15i = 0;
  float4 GLF_live15d = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int GLF_live15i_1 = 0;
  GLF_live15c = float4(0.0f, 0.0f, 0.0f, 0.0f);
  GLF_live15i = 0;
  {
    for(; (GLF_live15i < 4); GLF_live15i = (GLF_live15i + 1)) {
      if ((GLF_live15i >= 3)) {
        break;
      }
      const float x_49 = GLF_live15c.y;
      if ((x_49 >= 1.0f)) {
        set_float4(GLF_live15c, GLF_live15i, 1.0f);
      }
    }
  }
  GLF_live15d = float4(0.0f, 0.0f, 0.0f, 0.0f);
  GLF_live15i_1 = 0;
  {
    for(; (GLF_live15i_1 < 4); GLF_live15i_1 = (GLF_live15i_1 + 1)) {
      if ((GLF_live15i_1 >= 3)) {
        break;
      }
      const float x_64 = GLF_live15d.y;
      if ((x_64 >= 1.0f)) {
        set_float4(GLF_live15d, GLF_live15i_1, 1.0f);
      }
    }
  }
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
