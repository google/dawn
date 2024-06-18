SKIP: FAILED

void f() {
  float2[2] v = arr;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:2:14: error: brackets are not allowed here; to declare an array, place the brackets after the name
  float2[2] v = arr;
        ~~~  ^
             [2]
hlsl.hlsl:2:17: error: use of undeclared identifier 'arr'
  float2[2] v = arr;
                ^

