SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  int x = u;
  s = x;
}

DXC validation failure:
hlsl.hlsl:3:11: error: use of undeclared identifier 'u'
  int x = u;
          ^
hlsl.hlsl:4:3: error: use of undeclared identifier 's'
  s = x;
  ^

