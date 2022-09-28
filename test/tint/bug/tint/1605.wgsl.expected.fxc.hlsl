SKIP: FAILED

cbuffer cbuffer_b : register(b0, space0) {
  uint4 b[1];
};

bool func_3() {
  {
    [loop] for(int i = 0; (i < asint(b[0].x)); i = (i + 1)) {
      {
        [loop] for(int j = -1; (j == 1); j = (j + 1)) {
          return false;
        }
      }
    }
  }
  return false;
}

[numthreads(1, 1, 1)]
void main() {
  func_3();
  return;
}
FXC validation failure:
C:\src\dawn\test\tint\Shader@0x0000026A74A16540(9,16-53): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\dawn\test\tint\Shader@0x0000026A74A16540(9,16-53): warning X3551: infinite loop detected - loop writes no values
C:\src\dawn\test\tint\Shader@0x0000026A74A16540(9,16-53): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\dawn\test\tint\Shader@0x0000026A74A16540(9,16-53): warning X3551: infinite loop detected - loop writes no values
C:\src\dawn\test\tint\Shader@0x0000026A74A16540(9,16-53): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\dawn\test\tint\Shader@0x0000026A74A16540(9,16-53): warning X3551: infinite loop detected - loop writes no values
C:\src\dawn\test\tint\Shader@0x0000026A74A16540(9,16-53): warning X3557: loop only executes for 0 iteration(s), consider removing [loop]
C:\src\dawn\test\tint\Shader@0x0000026A74A16540(9,16-53): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\dawn\test\tint\Shader@0x0000026A74A16540(5,11-13): error X4555: cannot use casts on l-values

