struct S {
  a : i32,
}

@group(0) @binding(0) var<storage, read_write> v : S;

@compute @workgroup_size(1)
fn foo() {
  v.a <<= 2;
}
