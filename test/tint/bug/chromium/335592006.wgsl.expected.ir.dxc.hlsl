SKIP: FAILED


void f() {
  int a = 1;
  int(a);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:4:7: error: redefinition of 'a'
  int(a);
      ^
hlsl.hlsl:3:7: note: previous definition is here
  int a = 1;
      ^


tint executable returned error: exit status 1
