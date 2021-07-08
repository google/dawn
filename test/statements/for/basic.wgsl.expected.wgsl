fn some_loop_body() {
}

fn f() {
  {
    var i : i32 = 0;
    loop {
      if (!((i < 5))) {
        break;
      }
      some_loop_body();

      continuing {
        i = (i + 1);
      }
    }
  }
}
