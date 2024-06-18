SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  int[3] zero = (int[3])0;
  int[3] init = {1, 2, 3};
}

DXC validation failure:
hlsl.hlsl:3:14: error: brackets are not allowed here; to declare an array, place the brackets after the name
  int[3] zero = (int[3])0;
     ~~~     ^
             [3]
hlsl.hlsl:4:14: error: brackets are not allowed here; to declare an array, place the brackets after the name
  int[3] init = {1, 2, 3};
     ~~~     ^
             [3]

