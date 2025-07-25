#version 310 es

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  int inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int x = 42;
  int empty[4] = int[4](0, 0, 0, 0);
  int nonempty[4] = int[4](1, 2, 3, 4);
  uint v_1 = uint(x);
  int nonempty_with_expr[4] = int[4](1, x, int((v_1 + uint(1))), nonempty[3u]);
  int nested_empty[2][3][4] = int[2][3][4](int[3][4](int[4](0, 0, 0, 0), int[4](0, 0, 0, 0), int[4](0, 0, 0, 0)), int[3][4](int[4](0, 0, 0, 0), int[4](0, 0, 0, 0), int[4](0, 0, 0, 0)));
  int nested_nonempty[2][3][4] = int[2][3][4](int[3][4](int[4](1, 2, 3, 4), int[4](5, 6, 7, 8), int[4](9, 10, 11, 12)), int[3][4](int[4](13, 14, 15, 16), int[4](17, 18, 19, 20), int[4](21, 22, 23, 24)));
  uint v_2 = uint(x);
  int v_3[4] = int[4](1, 2, x, int((v_2 + uint(1))));
  uint v_4 = uint(nonempty[3u]);
  int nested_nonempty_with_expr[2][3][4] = int[2][3][4](int[3][4](v_3, int[4](5, 6, nonempty[2u], int((v_4 + uint(1)))), nonempty), nested_nonempty[1u]);
  int subexpr_empty = 0;
  int subexpr_nonempty = 3;
  uint v_5 = uint(x);
  int subexpr_nonempty_with_expr = int[4](1, x, int((v_5 + uint(1))), nonempty[3u])[2u];
  int subexpr_nested_empty[4] = int[4](0, 0, 0, 0);
  int subexpr_nested_nonempty[4] = int[4](5, 6, 7, 8);
  uint v_6 = uint(x);
  int subexpr_nested_nonempty_with_expr[4] = int[2][4](int[4](1, x, int((v_6 + uint(1))), nonempty[3u]), nested_nonempty[1u][2u])[1u];
  uint v_7 = uint(empty[0u]);
  uint v_8 = uint(int((v_7 + uint(nonempty[0u]))));
  uint v_9 = uint(int((v_8 + uint(nonempty_with_expr[0u]))));
  uint v_10 = uint(int((v_9 + uint(nested_empty[0u][0u][0u]))));
  uint v_11 = uint(int((v_10 + uint(nested_nonempty[0u][0u][0u]))));
  uint v_12 = uint(int((v_11 + uint(nested_nonempty_with_expr[0u][0u][0u]))));
  uint v_13 = uint(int((v_12 + uint(subexpr_empty))));
  uint v_14 = uint(int((v_13 + uint(subexpr_nonempty))));
  uint v_15 = uint(int((v_14 + uint(subexpr_nonempty_with_expr))));
  uint v_16 = uint(int((v_15 + uint(subexpr_nested_empty[0u]))));
  uint v_17 = uint(int((v_16 + uint(subexpr_nested_nonempty[0u]))));
  v.inner = int((v_17 + uint(subexpr_nested_nonempty_with_expr[0u])));
}
