
static uint3 localId = (0u).xxx;
static uint localIndex = 0u;
static uint3 globalId = (0u).xxx;
static uint3 numWorkgroups = (0u).xxx;
static uint3 workgroupId = (0u).xxx;
uint globalId2Index() {
  return globalId.x;
}

[numthreads(1, 1, 1)]
void main() {
  vector<float16_t, 4> a = (float16_t(0.0h)).xxxx;
  float16_t b = float16_t(1.0h);
  a[int(0)] = (a.x + b);
}

