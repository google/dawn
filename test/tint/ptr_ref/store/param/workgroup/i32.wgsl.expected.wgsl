enable chromium_experimental_full_ptr_parameters;

var<workgroup> S : i32;

fn func(pointer : ptr<workgroup, i32>) {
  *(pointer) = 42;
}

@compute @workgroup_size(1)
fn main() {
  func(&(S));
}
