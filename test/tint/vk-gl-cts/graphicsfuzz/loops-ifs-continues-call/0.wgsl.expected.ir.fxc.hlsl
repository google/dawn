SKIP: FAILED

struct BinarySearchObject {
  int prime_numbers[10];
};

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_8 : register(b0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (-2147483648))) : (2147483647));
}

int binarySearch_struct_BinarySearchObject_i1_10_1_(inout BinarySearchObject obj) {
  int m = 0;
  {
    while(true) {
      float x_91 = asfloat(x_8[0u].x);
      if ((x_91 > 1.0f)) {
      } else {
        break;
      }
      float x_95 = asfloat(x_8[0u].x);
      m = tint_f32_to_i32(x_95);
      int x_14 = m;
      int x_15 = obj.prime_numbers[x_14];
      if ((x_15 == 1)) {
        return 1;
      }
      {
      }
      continue;
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
    while(true) {
      int x_16 = i;
      if ((x_16 < 10)) {
      } else {
        break;
      }
      int x_17 = i;
      if ((x_17 != 3)) {
        int x_18 = i;
        float x_67 = asfloat(x_8[0u].x);
        if (((x_18 - tint_f32_to_i32(x_67)) == 4)) {
          int x_21 = i;
          obj_1.prime_numbers[x_21] = 11;
        } else {
          int x_22 = i;
          if ((x_22 == 6)) {
            int x_23 = i;
            obj_1.prime_numbers[x_23] = 17;
          }
          {
            int x_24 = i;
            i = (x_24 + 1);
          }
          continue;
        }
      }
      {
        while(true) {
          {
            float x_82 = asfloat(x_8[0u].y);
            if (!((0.0f > x_82))) { break; }
          }
          continue;
        }
      }
      {
        int x_24 = i;
        i = (x_24 + 1);
      }
      continue;
    }
  }
  BinarySearchObject v = obj_1;
  BinarySearchObject x_84 = v;
  param = x_84;
  int x_26 = binarySearch_struct_BinarySearchObject_i1_10_1_(param);
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
}

main_out main_inner() {
  main_1();
  main_out v_1 = {x_GLF_color};
  return v_1;
}

main_outputs main() {
  main_out v_2 = main_inner();
  main_outputs v_3 = {v_2.x_GLF_color_1};
  return v_3;
}

FXC validation failure:
C:\src\dawn\Shader@0x0000019ADAC81550(79,9-19): warning X3557: loop doesn't seem to do anything, forcing loop to unroll
C:\src\dawn\Shader@0x0000019ADAC81550(79,9-19): warning X3557: loop doesn't seem to do anything, forcing loop to unroll
C:\src\dawn\Shader@0x0000019ADAC81550(79,9-19): warning X3557: loop doesn't seem to do anything, forcing loop to unroll
C:\src\dawn\Shader@0x0000019ADAC81550(64,11-35): error X3500: array reference cannot be used as an l-value; not natively addressable
C:\src\dawn\Shader@0x0000019ADAC81550(52,5-15): error X3511: forced to unroll loop, but unrolling failed.

