static int x_GLF_global_loop_count = 0;
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  x_GLF_global_loop_count = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_27 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  switch(x_27) {
    case 0: {
      if (true) {
        const int x_34 = asint(x_6[1].x);
        const float x_35 = float(x_34);
        x_GLF_color = float4(x_35, x_35, x_35, x_35);
        return;
      }
      /* fallthrough */
      {
        if (true) {
          const uint scalar_offset_1 = ((16u * uint(0))) / 4;
          const int x_40 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
          const int x_43 = asint(x_6[1].x);
          const int x_46 = asint(x_6[1].x);
          const uint scalar_offset_2 = ((16u * uint(0))) / 4;
          const int x_49 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
          x_GLF_color = float4(float(x_40), float(x_43), float(x_46), float(x_49));
          if (false) {
            const uint scalar_offset_3 = ((16u * uint(0))) / 4;
            const int x_55 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
            const float x_56 = float(x_55);
            x_GLF_color = float4(x_56, x_56, x_56, x_56);
            while (true) {
              x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
              if (false) {
                return;
              }
              if (true) {
                return;
              }
              {
                if ((x_GLF_global_loop_count < 100)) {
                } else {
                  break;
                }
              }
            }
          }
        }
      }
      break;
    }
    case 1: {
      if (true) {
        const uint scalar_offset_4 = ((16u * uint(0))) / 4;
        const int x_40 = asint(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
        const int x_43 = asint(x_6[1].x);
        const int x_46 = asint(x_6[1].x);
        const uint scalar_offset_5 = ((16u * uint(0))) / 4;
        const int x_49 = asint(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
        x_GLF_color = float4(float(x_40), float(x_43), float(x_46), float(x_49));
        if (false) {
          const uint scalar_offset_6 = ((16u * uint(0))) / 4;
          const int x_55 = asint(x_6[scalar_offset_6 / 4][scalar_offset_6 % 4]);
          const float x_56 = float(x_55);
          x_GLF_color = float4(x_56, x_56, x_56, x_56);
          while (true) {
            x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
            if (false) {
              return;
            }
            if (true) {
              return;
            }
            {
              if ((x_GLF_global_loop_count < 100)) {
              } else {
                break;
              }
            }
          }
        }
      }
      break;
    }
    default: {
      break;
    }
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
