SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  int[3] v0 = zero;
  int[3] v1 = init;
}

DXC validation failure:
hlsl.hlsl:3:12: error: brackets are not allowed here; to declare an array, place the brackets after the name
  int[3] v0 = zero;
     ~~~   ^
           [3]
hlsl.hlsl:3:15: error: use of undeclared identifier 'zero'
  int[3] v0 = zero;
              ^
hlsl.hlsl:4:12: error: brackets are not allowed here; to declare an array, place the brackets after the name
  int[3] v1 = init;
     ~~~   ^
           [3]
hlsl.hlsl:4:15: error: use of undeclared identifier 'init'
  int[3] v1 = init;
              ^

