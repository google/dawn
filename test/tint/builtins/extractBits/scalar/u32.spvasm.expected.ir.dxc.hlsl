
void f_1() {
  uint v = 0u;
  uint offset_1 = 0u;
  uint count = 0u;
  uint v_1 = v;
  uint v_2 = count;
  uint v_3 = min(offset_1, 32u);
  uint v_4 = (32u - min(32u, (v_3 + v_2)));
  uint v_5 = (((v_4 < 32u)) ? ((v_1 << uint(v_4))) : (0u));
  uint x_11 = ((((v_4 + v_3) < 32u)) ? ((v_5 >> uint((v_4 + v_3)))) : (((v_5 >> 31u) >> 1u)));
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
}

