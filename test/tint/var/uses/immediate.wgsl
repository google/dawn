var<immediate> a : i32;
var<immediate> b : i32;
var<immediate> c : i32; // unused

fn uses_a() {
  let foo = a;
}

fn uses_uses_a() {
  uses_a();
}

fn uses_b() {
  let foo = b;
}

@compute @workgroup_size(1)
fn main1() {
  uses_a();
}

@compute @workgroup_size(1)
fn main2() {
  uses_uses_a();
}

@compute @workgroup_size(1)
fn main3() {
  uses_b();
}

@compute @workgroup_size(1)
fn main4() {
}
