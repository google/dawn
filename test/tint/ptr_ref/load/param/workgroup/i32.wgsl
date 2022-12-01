enable chromium_experimental_full_ptr_parameters;

var<workgroup> S : i32;

fn func(pointer : ptr<workgroup, i32>) -> i32 {
  return *pointer;
}

@compute @workgroup_size(1)
fn main() {
  let r = func(&S);
}
