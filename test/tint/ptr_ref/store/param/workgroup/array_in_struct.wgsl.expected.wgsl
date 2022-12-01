enable chromium_experimental_full_ptr_parameters;

struct str {
  arr : array<i32, 4>,
}

var<workgroup> S : str;

fn func(pointer : ptr<workgroup, array<i32, 4>>) {
  *(pointer) = array<i32, 4>();
}

@compute @workgroup_size(1)
fn main() {
  func(&(S.arr));
}
