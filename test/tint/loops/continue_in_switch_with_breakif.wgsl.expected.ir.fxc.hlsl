SKIP: FAILED


[numthreads(1, 1, 1)]
void f() {
  int i = 0;
  {
    while(true) {
      switch(i) {
        case 0:
        {
          {
            i = (i + 1);
            if ((i >= 4)) { break; }
          }
          continue;
        }
        default:
        {
          break;
        }
      }
      {
        i = (i + 1);
        if ((i >= 4)) { break; }
      }
      continue;
    }
  }
}

FXC validation failure:
<scrubbed_path>(14,11-19): error X3708: continue cannot be used in a switch


tint executable returned error: exit status 1
