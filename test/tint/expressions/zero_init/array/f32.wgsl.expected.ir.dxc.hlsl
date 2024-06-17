SKIP: FAILED

void f() {
  float[4] v = (float[4])0;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:2:13: error: brackets are not allowed here; to declare an array, place the brackets after the name
  float[4] v = (float[4])0;
       ~~~  ^
            [4]

