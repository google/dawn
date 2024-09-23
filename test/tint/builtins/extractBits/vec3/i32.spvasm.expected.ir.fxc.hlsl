
void f_1() {
  int3 v = (int(0)).xxx;
  uint offset_1 = 0u;
  uint count = 0u;
  int3 v_1 = v;
  uint v_2 = count;
  uint v_3 = min(offset_1, 32u);
  uint v_4 = (32u - min(32u, (v_3 + v_2)));
  int3 v_5 = (((v_4 < 32u)) ? ((v_1 << uint3((v_4).xxx))) : ((int(0)).xxx));
  int3 x_15 = ((((v_4 + v_3) < 32u)) ? ((v_5 >> uint3(((v_4 + v_3)).xxx))) : (((v_5 >> (31u).xxx) >> (1u).xxx)));
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
}

