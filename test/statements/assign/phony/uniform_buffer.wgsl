[[block]]
struct S {
    i : i32;
};

[[binding(0), group(0)]] var<uniform> u : S;

[[stage(compute), workgroup_size(1)]]
fn main() {
    _ = u;
    _ = u.i;
}
