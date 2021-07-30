var<private> myvar : vec4<u32>;

var<private> x_10 : vec4<u32>;

fn main_1() {
  let x_11 : vec4<u32> = x_10;
  let a_dynamic_index : u32 = x_11.z;
  myvar[a_dynamic_index] = 42u;
  return;
}

[[stage(fragment)]]
fn main() {
  main_1();
}
