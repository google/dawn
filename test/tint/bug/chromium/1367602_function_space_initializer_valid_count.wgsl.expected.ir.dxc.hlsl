SKIP: FAILED

void f() {
  bool[65535] v = (bool[65535])0;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:2:16: error: brackets are not allowed here; to declare an array, place the brackets after the name
  bool[65535] v = (bool[65535])0;
      ~~~~~~~  ^
               [65535]

