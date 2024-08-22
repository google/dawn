SKIP: FAILED

struct InnerS {
  int v;
};

struct S1 {
  InnerS a2[8];
};

struct OuterS {
  S1 a1[8];
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  OuterS s = (OuterS)0;
  InnerS v_1 = v;
  s.a1[uniforms[0u].x].a2[uniforms[0u].y] = v_1;
}

FXC validation failure:
<scrubbed_path>(22,3-22): error X3500: array reference cannot be used as an l-value; not natively addressable

