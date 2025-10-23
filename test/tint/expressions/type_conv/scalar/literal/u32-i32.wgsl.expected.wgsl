var<private> u : i32 = i32(u32(1u));

@compute @workgroup_size(1)
fn main() {
  _ = u;
}
