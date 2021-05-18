[[block]]
struct S {
  a : array<i32>;
};

[[group(0), binding(0)]] var<storage> G : [[access(read)]] S;

[[stage(compute)]]
fn main() {
}
