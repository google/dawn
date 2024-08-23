SKIP: FAILED


RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void main() {
  s.Store3(0u, asuint((1).xxx));
}


tint executable returned error: exit status 0xe0000001
