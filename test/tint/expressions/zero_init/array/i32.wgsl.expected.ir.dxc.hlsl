SKIP: FAILED

void f() {
  int[4] v = (int[4])0;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:2:11: error: brackets are not allowed here; to declare an array, place the brackets after the name
  int[4] v = (int[4])0;
     ~~~  ^
          [4]

