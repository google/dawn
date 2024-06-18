SKIP: FAILED

void main() {
  int[5] a = (int[5])0;
}

DXC validation failure:
hlsl.hlsl:2:11: error: brackets are not allowed here; to declare an array, place the brackets after the name
  int[5] a = (int[5])0;
     ~~~  ^
          [5]

