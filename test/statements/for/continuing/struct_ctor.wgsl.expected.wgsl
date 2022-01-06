struct S {
  i : i32;
};

fn f() {
  for(var i = 0; ; i = (i + S(1).i)) {
  }
}
