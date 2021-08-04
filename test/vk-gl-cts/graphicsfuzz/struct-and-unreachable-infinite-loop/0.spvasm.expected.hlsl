struct BST {
  int data;
  int leftIndex;
  int rightIndex;
};

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void makeTreeNode_struct_BST_i1_i1_i11_(inout BST tree) {
  tree.rightIndex = 1;
  return;
}

void main_1() {
  BST tree_1[10] = (BST[10])0;
  BST param = (BST)0;
  param = tree_1[0u];
  makeTreeNode_struct_BST_i1_i1_i11_(param);
  const BST x_40 = param;
  BST x_42_1[10] = tree_1;
  x_42_1[0u] = x_40;
  tree_1 = x_42_1;
  if ((tree_1[0u].rightIndex == 0)) {
    while (true) {
    }
    return;
  }
  x_GLF_color = float4(float(tree_1[0u].rightIndex), 0.0f, 0.0f, 1.0f);
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
