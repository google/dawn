SKIP: FAILED

void main() {
  i = (i - 1);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:2:3: error: use of undeclared identifier 'i'
  i = (i - 1);
  ^
hlsl.hlsl:2:8: error: use of undeclared identifier 'i'
  i = (i - 1);
       ^

