fn f(a: i32, b: i32, c: i32) -> i32 {
    return a * b + c;
}

[[stage(compute), workgroup_size(1)]]
fn main() {
    ignore(f(1, 2, 3));
}
