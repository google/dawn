fn f(cond : bool) {
  if (cond) {
    discard;
    return;
  }
}

@fragment
fn main() {
    f(false);
}
