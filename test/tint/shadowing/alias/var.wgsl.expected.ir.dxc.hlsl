SKIP: FAILED

void f() {
  int a = 0;
  int b = a;
  int a = 0;
  int b = a;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:4:7: error: redefinition of 'a'
  int a = 0;
      ^
hlsl.hlsl:2:7: note: previous definition is here
  int a = 0;
      ^
hlsl.hlsl:5:7: error: redefinition of 'b'
  int b = a;
      ^
hlsl.hlsl:3:7: note: previous definition is here
  int b = a;
      ^

