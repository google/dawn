SKIP: FAILED

struct OuterS {
  float2x4 m1;
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  OuterS s1 = (OuterS)0;
  s1.m1[uniforms[0u].x] = (1.0f).xxxx;
  s1.m1[uniforms[0u].x][uniforms[0u].x] = 1.0f;
}

FXC validation failure:
<scrubbed_path>(12,3-23): error X3500: array reference cannot be used as an l-value; not natively addressable


tint executable returned error: exit status 1
