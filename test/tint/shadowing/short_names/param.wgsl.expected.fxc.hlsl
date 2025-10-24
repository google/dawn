
void f(float3 vec3f) {
  float3 b = vec3f;
}

[numthreads(1, 1, 1)]
void main() {
  f((0.0f).xxx);
}

