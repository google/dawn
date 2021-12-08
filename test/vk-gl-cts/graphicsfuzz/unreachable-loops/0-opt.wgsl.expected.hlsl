SKIP: FAILED

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[1];
};

void main_1() {
  int m = 0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  const float x_30 = asfloat(x_5[0].x);
  const float x_32 = asfloat(x_5[0].y);
  if ((x_30 > x_32)) {
    [loop] while (true) {
      {
        if (false) {
        } else {
          break;
        }
      }
    }
    m = 1;
    [loop] while (true) {
      if (true) {
      } else {
        break;
      }
      x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    }
  }
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x00000229EC6CE4C0(12,12-23): warning X3557: loop only executes for 0 iteration(s), consider removing [loop]
C:\src\tint\test\Shader@0x00000229EC6CE4C0(12,12-23): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\tint\test\Shader@0x00000229EC6CE4C0(21,19-22): error X3696: infinite loop detected - loop never exits

