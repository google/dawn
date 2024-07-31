SKIP: FAILED

struct InnerS {
  int v;
};

struct OuterS {
  InnerS a1[8];
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
void f(inout OuterS p) {
  InnerS v = (InnerS)0;
  InnerS v_1 = v;
  p.a1[uniforms[0u].x] = v_1;
}

[numthreads(1, 1, 1)]
void main() {
  OuterS s1 = (OuterS)0;
  f(s1);
}

FXC validation failure:
c:\src\dawn\Shader@0x00000225DC940170(16,3-22): error X3500: array reference cannot be used as an l-value; not natively addressable

