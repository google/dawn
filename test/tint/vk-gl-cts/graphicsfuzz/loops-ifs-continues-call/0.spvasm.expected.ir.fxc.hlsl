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
      if ((asfloat(x_8[0u].x) > 1.0f)) {
      } else {
        break;
      }
      m = tint_f32_to_i32(asfloat(x_8[0u].x));
      if ((obj.prime_numbers[m] == 1)) {
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
      if ((i < 10)) {
      } else {
        break;
      }
      if ((i != 3)) {
        int v = i;
        if (((v - tint_f32_to_i32(asfloat(x_8[0u].x))) == 4)) {
          int x_21 = i;
          obj_1.prime_numbers[x_21] = 11;
        } else {
          if ((i == 6)) {
            int x_23 = i;
            obj_1.prime_numbers[x_23] = 17;
          }
          {
            i = (i + 1);
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
        i = (i + 1);
      }
      continue;
    }
  }
  BinarySearchObject v_1 = obj_1;
  param = v_1;
  int x_26 = binarySearch_struct_BinarySearchObject_i1_10_1_(param);
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
}

main_out main_inner() {
  main_1();
  main_out v_2 = {x_GLF_color};
  return v_2;
}

main_outputs main() {
  main_out v_3 = main_inner();
  main_outputs v_4 = {v_3.x_GLF_color_1};
  return v_4;
}

FXC validation failure:
c:\src\dawn\Shader@0x00000122B9423EA0(70,9-19): warning X3557: loop doesn't seem to do anything, forcing loop to unroll
c:\src\dawn\Shader@0x00000122B9423EA0(70,9-19): warning X3557: loop doesn't seem to do anything, forcing loop to unroll
c:\src\dawn\Shader@0x00000122B9423EA0(70,9-19): warning X3557: loop doesn't seem to do anything, forcing loop to unroll
c:\src\dawn\Shader@0x00000122B9423EA0(57,11-35): error X3500: array reference cannot be used as an l-value; not natively addressable
c:\src\dawn\Shader@0x00000122B9423EA0(48,5-15): error X3511: forced to unroll loop, but unrolling failed.

