SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  int2 x = u;
  s = x;
}

DXC validation failure:
hlsl.hlsl:3:12: error: use of undeclared identifier 'u'
  int2 x = u;
           ^
hlsl.hlsl:4:3: error: use of undeclared identifier 's'
  s = x;
  ^

