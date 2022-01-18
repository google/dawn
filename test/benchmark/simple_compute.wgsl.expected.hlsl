RWByteAddressBuffer buffer : register(u0, space0);

struct tint_symbol_1 {
  uint3 id : SV_DispatchThreadID;
};

void main_inner(uint3 id) {
  buffer.Store((4u * id.x), asuint((asint(buffer.Load((4u * id.x))) + 1)));
}

[numthreads(1, 2, 3)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.id);
  return;
}
