struct S {
  algo : u32;
};

struct S_1 {
  rithm : u32;
};

fn main_1() {
  var var0 : S;
  var var1 : S_1;
  let x_1 : S = var0;
  var x_2_1 : S = x_1;
  x_2_1.algo = 10u;
  let x_2 : S = x_2_1;
  let x_3 : S_1 = var1;
  var x_4_1 : S_1 = x_3;
  x_4_1.rithm = 11u;
  let x_4 : S_1 = x_4_1;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
