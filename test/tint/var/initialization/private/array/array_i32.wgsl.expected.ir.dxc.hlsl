SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  int[2][3] v0 = zero;
  int[2][3] v1 = init;
}

DXC validation failure:
hlsl.hlsl:3:15: error: brackets are not allowed here; to declare an array, place the brackets after the name
  int[2][3] v0 = zero;
     ~~~~~~   ^
              [2][3]
hlsl.hlsl:3:18: error: use of undeclared identifier 'zero'
  int[2][3] v0 = zero;
                 ^
hlsl.hlsl:4:15: error: brackets are not allowed here; to declare an array, place the brackets after the name
  int[2][3] v1 = init;
     ~~~~~~   ^
              [2][3]
hlsl.hlsl:4:18: error: use of undeclared identifier 'init'
  int[2][3] v1 = init;
                 ^

