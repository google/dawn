SKIP: FAILED

static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float array0[3] = (float[3])0;
static float array1[3] = (float[3])0;
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int q = 0;
  int i = 0;
  int c = 0;
  q = 0;
  const float x_55 = gl_FragCoord.x;
  i = (int(x_55) % 3);
  c = 0;
  {
    [loop] for(; (c < 3); c = (c + 1)) {
      array0[c] = 0.0f;
      array1[c] = 0.0f;
      const float x_65 = asfloat(x_11[0].x);
      switch((int(x_65) + q)) {
        case 51: {
          [loop] while (true) {
            if (true) {
            } else {
              break;
            }
          }
          array0[c] = 1.0f;
          /* fallthrough */
          {
            array1[0] = 1.0f;
            array1[c] = 1.0f;
          }
          break;
        }
        case 61: {
          array1[0] = 1.0f;
          array1[c] = 1.0f;
          break;
        }
        case 0: {
          q = 61;
          break;
        }
        default: {
          break;
        }
      }
    }
  }
  const float x_79 = array1[i];
  const float x_81 = array0[i];
  const float x_83 = array0[i];
  x_GLF_color = float4(x_79, x_81, x_83, 1.0f);
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x0000012C749BFE70(15,8-20): warning X3556: integer modulus may be much slower, try using uints if possible.
C:\src\tint\test\Shader@0x0000012C749BFE70(24,18-29): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\tint\test\Shader@0x0000012C749BFE70(24,18-29): warning X3551: infinite loop detected - loop writes no values
C:\src\tint\test\Shader@0x0000012C749BFE70(24,18-29): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\tint\test\Shader@0x0000012C749BFE70(24,18-29): warning X3551: infinite loop detected - loop writes no values
C:\src\tint\test\Shader@0x0000012C749BFE70(24,18-29): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\tint\test\Shader@0x0000012C749BFE70(24,18-29): warning X3551: infinite loop detected - loop writes no values
C:\src\tint\test\Shader@0x0000012C749BFE70(24,18-29): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\tint\test\Shader@0x0000012C749BFE70(24,18-29): warning X3551: infinite loop detected - loop writes no values
C:\src\tint\test\Shader@0x0000012C749BFE70(24,18-29): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\tint\test\Shader@0x0000012C749BFE70(24,18-29): warning X3551: infinite loop detected - loop writes no values
C:\src\tint\test\Shader@0x0000012C749BFE70(24,18-29): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\tint\test\Shader@0x0000012C749BFE70(24,18-29): warning X3551: infinite loop detected - loop writes no values
C:\src\tint\test\Shader@0x0000012C749BFE70(24,25-28): error X3696: infinite loop detected - loop never exits

