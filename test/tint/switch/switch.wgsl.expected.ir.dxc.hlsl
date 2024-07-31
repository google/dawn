
void a() {
  int a_1 = 0;
  switch(a_1) {
    case 0:
    {
      break;
    }
    case 1:
    {
      return;
    }
    default:
    {
      a_1 = (a_1 + 2);
      break;
    }
  }
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

