SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  int[2][3] zero = (int[2][3])0;
  int[2][3] init = {{1, 2, 3}, {4, 5, 6}};
}

DXC validation failure:
hlsl.hlsl:3:17: error: brackets are not allowed here; to declare an array, place the brackets after the name
  int[2][3] zero = (int[2][3])0;
     ~~~~~~     ^
                [2][3]
hlsl.hlsl:4:17: error: brackets are not allowed here; to declare an array, place the brackets after the name
  int[2][3] init = {{1, 2, 3}, {4, 5, 6}};
     ~~~~~~     ^
                [2][3]

