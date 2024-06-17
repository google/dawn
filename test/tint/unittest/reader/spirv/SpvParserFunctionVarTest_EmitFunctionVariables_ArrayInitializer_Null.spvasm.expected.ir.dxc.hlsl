SKIP: FAILED

void main_1() {
  uint[2] x_200 = (uint[2])0;
}

void main() {
  main_1();
}

DXC validation failure:
hlsl.hlsl:2:16: error: brackets are not allowed here; to declare an array, place the brackets after the name
  uint[2] x_200 = (uint[2])0;
      ~~~      ^
               [2]

