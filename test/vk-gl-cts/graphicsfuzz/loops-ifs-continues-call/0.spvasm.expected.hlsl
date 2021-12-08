SKIP: FAILED

struct BinarySearchObject {
  int prime_numbers[10];
};

cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int binarySearch_struct_BinarySearchObject_i1_10_1_(inout BinarySearchObject obj) {
  int m = 0;
  [loop] while (true) {
    const float x_91 = asfloat(x_8[0].x);
    if ((x_91 > 1.0f)) {
    } else {
      break;
    }
    const float x_95 = asfloat(x_8[0].x);
    m = int(x_95);
    const int x_15 = obj.prime_numbers[m];
    if ((x_15 == 1)) {
      return 1;
    }
  }
  return 1;
}

void main_1() {
  int i = 0;
  BinarySearchObject obj_1 = (BinarySearchObject)0;
  BinarySearchObject param = (BinarySearchObject)0;
  i = 0;
  {
    [loop] for(; (i < 10); i = (i + 1)) {
      if ((i != 3)) {
        const int x_18 = i;
        const float x_67 = asfloat(x_8[0].x);
        if (((x_18 - int(x_67)) == 4)) {
          obj_1.prime_numbers[i] = 11;
        } else {
          if ((i == 6)) {
            obj_1.prime_numbers[i] = 17;
          }
          continue;
        }
      }
      [loop] while (true) {
        {
          const float x_82 = asfloat(x_8[0].y);
          if ((0.0f > x_82)) {
          } else {
            break;
          }
        }
      }
    }
  }
  param = obj_1;
  const int x_26 = binarySearch_struct_BinarySearchObject_i1_10_1_(param);
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x0000016C5E617FA0(47,14-25): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\tint\test\Shader@0x0000016C5E617FA0(47,14-25): warning X3551: infinite loop detected - loop writes no values
C:\src\tint\test\Shader@0x0000016C5E617FA0(47,14-25): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\tint\test\Shader@0x0000016C5E617FA0(47,14-25): warning X3551: infinite loop detected - loop writes no values
C:\src\tint\test\Shader@0x0000016C5E617FA0(47,14-25): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\tint\test\Shader@0x0000016C5E617FA0(47,14-25): warning X3551: infinite loop detected - loop writes no values
C:\src\tint\test\Shader@0x0000016C5E617FA0(39,11-32): warning X3550: array reference cannot be used as an l-value; not natively addressable, forcing loop to unroll
C:\src\tint\test\Shader@0x0000016C5E617FA0(34,12-39): error X3531: can't unroll loops marked with loop attribute

