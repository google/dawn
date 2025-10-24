struct a { a : i32 };

fn f(a : a) {
  let b = a;
}

@compute @workgroup_size(1)
fn main() {
    f(a());
}
