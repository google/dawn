struct BST {
  data : i32;
  leftIndex : i32;
  rightIndex : i32;
};

var<private> x_GLF_color : vec4<f32>;

fn makeTreeNode_struct_BST_i1_i1_i11_(tree : ptr<function, BST>) {
  (*(tree)).rightIndex = 1;
  return;
}

fn main_1() {
  var tree_1 : array<BST, 10>;
  var param : BST;
  let x_37 : array<BST, 10> = tree_1;
  param = x_37[0u];
  makeTreeNode_struct_BST_i1_i1_i11_(&(param));
  let x_40 : BST = param;
  let x_41 : array<BST, 10> = tree_1;
  var x_42_1 : array<BST, 10> = x_41;
  x_42_1[0u] = x_40;
  let x_42 : array<BST, 10> = x_42_1;
  tree_1 = x_42;
  let x_43 : ptr<function, i32> = &(tree_1[0].rightIndex);
  let x_11 : array<BST, 10> = tree_1;
  if ((x_11[0u].rightIndex == 0)) {
    loop {
    }
    return;
  }
  let x_12 : array<BST, 10> = tree_1;
  x_GLF_color = vec4<f32>(f32(x_12[0u].rightIndex), 0.0, 0.0, 1.0);
  return;
}

struct main_out {
  [[location(0)]]
  x_GLF_color_1 : vec4<f32>;
};

[[stage(fragment)]]
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
