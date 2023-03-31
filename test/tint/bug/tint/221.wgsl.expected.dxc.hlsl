RWByteAddressBuffer b : register(u0);

uint tint_mod(uint lhs, uint rhs) {
  return (lhs % ((rhs == 0u) ? 1u : rhs));
}

[numthreads(1, 1, 1)]
void main() {
  uint i = 0u;
  while (true) {
    if ((i >= b.Load(0u))) {
      break;
    }
    const uint p_save = i;
    if ((tint_mod(i, 2u) == 0u)) {
      {
        b.Store((4u + (4u * p_save)), asuint((b.Load((4u + (4u * p_save))) * 2u)));
        i = (i + 1u);
      }
      continue;
    }
    b.Store((4u + (4u * p_save)), asuint(0u));
    {
      b.Store((4u + (4u * p_save)), asuint((b.Load((4u + (4u * p_save))) * 2u)));
      i = (i + 1u);
    }
  }
  return;
}
