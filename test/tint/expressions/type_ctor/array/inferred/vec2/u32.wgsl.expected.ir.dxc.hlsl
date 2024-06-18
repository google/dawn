SKIP: FAILED

void f() {
  uint2[2] v = arr;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:2:13: error: brackets are not allowed here; to declare an array, place the brackets after the name
  uint2[2] v = arr;
       ~~~  ^
            [2]
hlsl.hlsl:2:16: error: use of undeclared identifier 'arr'
  uint2[2] v = arr;
               ^

