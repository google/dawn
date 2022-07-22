const o : bool = false;

const j : bool = true;

@compute @workgroup_size(1)
fn main() {
  if ((o && j)) {
    _ = o;
  }
}
