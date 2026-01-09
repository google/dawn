var<workgroup> v : buffer<64>;

fn foo(p : ptr<workgroup, buffer<64>>) {
  let p2 = bufferView<vec2f>(p, 0);
  p2.x = 1.0;
}

@compute @workgroup_size(1)
fn main() {
  foo(&v);
}
