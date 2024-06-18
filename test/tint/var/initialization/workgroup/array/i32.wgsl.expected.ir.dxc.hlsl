SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  int[3] v = zero;
}

DXC validation failure:
hlsl.hlsl:3:11: error: brackets are not allowed here; to declare an array, place the brackets after the name
  int[3] v = zero;
     ~~~  ^
          [3]
hlsl.hlsl:3:14: error: use of undeclared identifier 'zero'
  int[3] v = zero;
             ^

