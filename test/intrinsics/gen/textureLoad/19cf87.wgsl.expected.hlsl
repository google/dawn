SKIP: FAILED



Validation Failure:
Texture2D arg_0 : register(t0, space1);

void textureLoad_19cf87() {
  float res = arg_0.Load(int3(0), 1);
}

void vertex_main() {
  textureLoad_19cf87();
  return;
}

void fragment_main() {
  textureLoad_19cf87();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_19cf87();
  return;
}


tint_ZX1vzT:4:31: error: too few elements in vector initialization (expected 3 elements, have 1)
  float res = arg_0.Load(int3(0), 1);
                              ^
tint_ZX1vzT:4:9: warning: implicit truncation of vector type [-Wconversion]
  float res = arg_0.Load(int3(0), 1);
        ^


tint_ZX1vzT:4:31: error: too few elements in vector initialization (expected 3 elements, have 1)
  float res = arg_0.Load(int3(0), 1);
                              ^
tint_ZX1vzT:4:9: warning: implicit truncation of vector type [-Wconversion]
  float res = arg_0.Load(int3(0), 1);
        ^


tint_ZX1vzT:4:31: error: too few elements in vector initialization (expected 3 elements, have 1)
  float res = arg_0.Load(int3(0), 1);
                              ^
tint_ZX1vzT:4:9: warning: implicit truncation of vector type [-Wconversion]
  float res = arg_0.Load(int3(0), 1);
        ^

