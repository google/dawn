enable chromium_experimental_full_ptr_parameters;

struct str {
  i : i32,
}

fn func(pointer : ptr<private, i32>) -> i32 {
  return *(pointer);
}

var<private> P : str;

@compute @workgroup_size(1)
fn main() {
  let r = func(&(P.i));
}
