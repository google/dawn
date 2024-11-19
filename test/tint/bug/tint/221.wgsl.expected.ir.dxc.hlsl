
RWByteAddressBuffer b : register(u0);
uint tint_mod_u32(uint lhs, uint rhs) {
  uint v = (((rhs == 0u)) ? (1u) : (rhs));
  return (lhs - ((lhs / v) * v));
}

[numthreads(1, 1, 1)]
void main() {
  uint i = 0u;
  {
    while(true) {
      if ((i >= b.Load(0u))) {
        break;
      }
      uint v_1 = (i * 4u);
      if ((tint_mod_u32(i, 2u) == 0u)) {
        {
          b.Store((4u + v_1), (b.Load((4u + v_1)) * 2u));
          i = (i + 1u);
        }
        continue;
      }
      b.Store((4u + v_1), 0u);
      {
        b.Store((4u + v_1), (b.Load((4u + v_1)) * 2u));
        i = (i + 1u);
      }
      continue;
    }
  }
}

