SKIP: FAILED

void main_1() {
  uint[5] x_1 = {10u, 20u, 3u, 4u, 5u};
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

DXC validation failure:
hlsl.hlsl:2:14: error: brackets are not allowed here; to declare an array, place the brackets after the name
  uint[5] x_1 = {10u, 20u, 3u, 4u, 5u};
      ~~~    ^
             [5]

