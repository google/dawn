
RWByteAddressBuffer a : register(u0);
void foo() {
  uint64_t v = uint64_t(0u);
  uint64_t v_1 = uint64_t(0u);
  a.InterlockedMin64(0u, ((v_1 << uint64_t(32u)) | v));
}

void main() {
  foo();
}

