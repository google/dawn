SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  float x = u;
  s = x;
}

DXC validation failure:
hlsl.hlsl:3:13: error: use of undeclared identifier 'u'
  float x = u;
            ^
hlsl.hlsl:4:3: error: use of undeclared identifier 's'
  s = x;
  ^

