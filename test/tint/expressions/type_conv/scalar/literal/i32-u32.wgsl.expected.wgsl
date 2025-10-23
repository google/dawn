var<private> u : u32 = u32(i32(1i));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
