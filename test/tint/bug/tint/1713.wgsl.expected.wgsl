const a = (1 << 30u);

const b = (a << 10u);

const c = (5000000000 << 3u);

@compute @workgroup_size(1)
fn main() {
  _ = b;
  _ = c;
}
