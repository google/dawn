SKIP: FAILED

void foo() {
  int[2] explicitStride = (int[2])0;
  int[2] implictStride = (int[2])0;
  implictStride = explicitStride;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:2:24: error: brackets are not allowed here; to declare an array, place the brackets after the name
  int[2] explicitStride = (int[2])0;
     ~~~               ^
                       [2]
hlsl.hlsl:3:23: error: brackets are not allowed here; to declare an array, place the brackets after the name
  int[2] implictStride = (int[2])0;
     ~~~              ^
                      [2]

