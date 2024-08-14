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


static uint nextIndex = 0u;
cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
uint getNextIndex() {
  nextIndex = (nextIndex + 1u);
  return nextIndex;
}

[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  OuterS s = (OuterS)0;
  uint v_1 = getNextIndex();
  InnerS v_2 = v;
  s.a1[v_1].a2[uniforms[0u].y] = v_2;
}

FXC validation failure:
C:\src\dawn\Shader@0x000002CD9F38DE90(29,3-30): error X3500: array reference cannot be used as an l-value; not natively addressable

