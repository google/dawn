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
  int nonempty_with_expr[4] = int[4](1, x, int((uint(x) + 1u)), nonempty[3]);
  int nested_empty[2][3][4] = int[2][3][4](int[3][4](int[4](0, 0, 0, 0), int[4](0, 0, 0, 0), int[4](0, 0, 0, 0)), int[3][4](int[4](0, 0, 0, 0), int[4](0, 0, 0, 0), int[4](0, 0, 0, 0)));
  int nested_nonempty[2][3][4] = int[2][3][4](int[3][4](int[4](1, 2, 3, 4), int[4](5, 6, 7, 8), int[4](9, 10, 11, 12)), int[3][4](int[4](13, 14, 15, 16), int[4](17, 18, 19, 20), int[4](21, 22, 23, 24)));
  int v_1[4] = int[4](1, 2, x, int((uint(x) + 1u)));
  int nested_nonempty_with_expr[2][3][4] = int[2][3][4](int[3][4](v_1, int[4](5, 6, nonempty[2], int((uint(nonempty[3]) + 1u))), nonempty), nested_nonempty[1]);
  int subexpr_empty = 0;
  int subexpr_nonempty = 3;
  int subexpr_nonempty_with_expr = int[4](1, x, int((uint(x) + 1u)), nonempty[3])[2];
  int subexpr_nested_empty[4] = int[4](0, 0, 0, 0);
  int subexpr_nested_nonempty[4] = int[4](5, 6, 7, 8);
  int subexpr_nested_nonempty_with_expr[4] = int[2][4](int[4](1, x, int((uint(x) + 1u)), nonempty[3]), nested_nonempty[1][2])[1];
  uint v_2 = uint(empty[0]);
  uint v_3 = uint(int((v_2 + uint(nonempty[0]))));
  uint v_4 = uint(int((v_3 + uint(nonempty_with_expr[0]))));
  uint v_5 = uint(int((v_4 + uint(nested_empty[0][0][0]))));
  uint v_6 = uint(int((v_5 + uint(nested_nonempty[0][0][0]))));
  uint v_7 = uint(int((v_6 + uint(nested_nonempty_with_expr[0][0][0]))));
  uint v_8 = uint(int((v_7 + uint(subexpr_empty))));
  uint v_9 = uint(int((v_8 + uint(subexpr_nonempty))));
  uint v_10 = uint(int((v_9 + uint(subexpr_nonempty_with_expr))));
  uint v_11 = uint(int((v_10 + uint(subexpr_nested_empty[0]))));
  uint v_12 = uint(int((v_11 + uint(subexpr_nested_nonempty[0]))));
  v.inner = int((v_12 + uint(subexpr_nested_nonempty_with_expr[0])));
}
