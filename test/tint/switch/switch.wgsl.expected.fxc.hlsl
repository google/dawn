
[numthreads(1, 1, 1)]
void a() {
  int a_1 = int(0);
  switch(a_1) {
    case int(0):
    {
      break;
    }
    case int(1):
    {
      return;
    }
    default:
    {
      a_1 = asint((asuint(a_1) + asuint(int(2))));
      break;
    }
  }
}

