var<private> x_1 : vec3<i32>;

fn main_1() {
  let x_13 : ptr<private, vec3<i32>> = &(x_1);
  let x_2 : vec3<i32> = *(x_13);
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main([[builtin(local_invocation_id)]] x_1_param : vec3<u32>) {
  x_1 = bitcast<vec3<i32>>(x_1_param);
  main_1();
}
