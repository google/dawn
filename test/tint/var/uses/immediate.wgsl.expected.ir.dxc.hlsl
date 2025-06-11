//
// main1
//

cbuffer cbuffer_a : register(b0) {
  uint4 a[1];
};
void uses_a() {
  int foo = asint(a[0u].x);
}

[numthreads(1, 1, 1)]
void main1() {
  uses_a();
}

//
// main2
//

cbuffer cbuffer_a : register(b0) {
  uint4 a[1];
};
void uses_a() {
  int foo = asint(a[0u].x);
}

void uses_uses_a() {
  uses_a();
}

[numthreads(1, 1, 1)]
void main2() {
  uses_uses_a();
}

//
// main3
//

cbuffer cbuffer_b : register(b0) {
  uint4 b[1];
};
void uses_b() {
  int foo = asint(b[0u].x);
}

[numthreads(1, 1, 1)]
void main3() {
  uses_b();
}

//
// main4
//

[numthreads(1, 1, 1)]
void main4() {
}

