SKIP: FAILED

struct InnerS {
  int v;
};

struct S1 {
  InnerS a[8];
};

struct OuterS {
  S1 s2;
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  OuterS s1 = (OuterS)0;
  InnerS v_1 = v;
  s1.s2.a[uniforms[0u].x] = v_1;
}

FXC validation failure:
<scrubbed_path>(22,3-25): error X3500: array reference cannot be used as an l-value; not natively addressable

