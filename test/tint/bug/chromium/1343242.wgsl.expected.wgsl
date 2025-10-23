var<private> o = bool(~(1));

@compute @workgroup_size(1)
fn main() {
  _ = o;
}
