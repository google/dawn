fn some_loop_body() {
}

fn f() {
  var j : i32;
  {
    var i : i32 = 0;
    loop {
      if (!(((i < 5) && (j < 10)))) {
        break;
      }
      some_loop_body();
      j = (i * 30);

      continuing {
        i = (i + 1);
      }
    }
  }
}
