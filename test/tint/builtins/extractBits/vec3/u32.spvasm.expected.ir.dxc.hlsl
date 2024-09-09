
void f_1() {
  uint3 v = (0u).xxx;
  uint offset_1 = 0u;
  uint count = 0u;
  uint3 v_1 = v;
  uint v_2 = count;
  uint v_3 = min(offset_1, 32u);
  uint v_4 = (32u - min(32u, (v_3 + v_2)));
  uint3 v_5 = (((v_4 < 32u)) ? ((v_1 << uint3((v_4).xxx))) : ((0u).xxx));
  uint3 x_14 = ((((v_4 + v_3) < 32u)) ? ((v_5 >> uint3(((v_4 + v_3)).xxx))) : (((v_5 >> (31u).xxx) >> (1u).xxx)));
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
}

