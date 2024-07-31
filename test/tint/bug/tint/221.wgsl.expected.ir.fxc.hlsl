
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
      uint v_1 = i;
      if ((v_1 >= b.Load(0u))) {
        break;
      }
      uint v_2 = (uint(i) * 4u);
      if ((tint_mod_u32(i, 2u) == 0u)) {
        {
          b.Store((4u + v_2), (b.Load((4u + v_2)) * 2u));
          i = (i + 1u);
        }
        continue;
      }
      b.Store((4u + v_2), 0u);
      {
        b.Store((4u + v_2), (b.Load((4u + v_2)) * 2u));
        i = (i + 1u);
      }
      continue;
    }
  }
}

