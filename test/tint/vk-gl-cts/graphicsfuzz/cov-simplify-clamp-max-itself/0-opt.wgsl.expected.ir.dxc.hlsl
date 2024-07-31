SKIP: FAILED


struct buf0 {
  sequence : vec4<i32>,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : vec4<i32>;
  var i : i32;
  var sum : i32;
  a = vec4<i32>(0, 0, 0, 0);
  i = 0;
  loop {
    let x_40 : i32 = i;
    let x_42 : i32 = x_7.sequence.w;
    if ((x_40 < (x_42 + 1))) {
    } else {
      break;
    }
    let x_46 : i32 = i;
    let x_48 : i32 = x_7.sequence.x;
    let x_49 : i32 = i;
    let x_52 : i32 = x_7.sequence[clamp(x_46, x_48, x_49)];
    if ((x_52 == 1)) {
      let x_57 : i32 = i;
      a[x_57] = 5;
    } else {
      let x_59 : i32 = i;
      let x_60 : i32 = i;
      a[x_59] = x_60;
    }

    continuing {
      let x_62 : i32 = i;
      i = (x_62 + 1);
    }
  }
  let x_65 : i32 = a.x;
  let x_67 : i32 = a.y;
  let x_70 : i32 = a.z;
  let x_73 : i32 = a.w;
  sum = (((x_65 + x_67) + x_70) + x_73);
  let x_75 : i32 = sum;
  if ((x_75 == 10)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}

Failed to generate: :49:9 error: binary: no matching overload for 'operator * (i32, u32)'

9 candidate operators:
 • 'operator * (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (vecN<T>  ✗ , T  ✓ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✗ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'f32' or 'f16'

        %25:u32 = mul %24, 4u
        ^^^^^^^^^^^^^^^^^^^^^

:22:7 note: in block
      $B3: {  # body
      ^^^

note: # Disassembly
buf0 = struct @align(16) {
  sequence:vec4<i32> @offset(0)
}

main_out = struct @align(16) {
  x_GLF_color_1:vec4<f32> @offset(0), @location(0)
}

$B1: {  # root
  %x_7:ptr<uniform, array<vec4<u32>, 1>, read> = var @binding_point(0, 0)
  %x_GLF_color:ptr<private, vec4<f32>, read_write> = var
}

%main_1 = func():void {
  $B2: {
    %a:ptr<function, vec4<i32>, read_write> = var
    %i:ptr<function, i32, read_write> = var
    %sum:ptr<function, i32, read_write> = var
    store %a, vec4<i32>(0i)
    store %i, 0i
    loop [b: $B3, c: $B4] {  # loop_1
      $B3: {  # body
        %7:i32 = load %i
        %x_40:i32 = let %7
        %9:ptr<uniform, vec4<u32>, read> = access %x_7, 0u
        %10:u32 = load_vector_element %9, 3u
        %11:i32 = bitcast %10
        %x_42:i32 = let %11
        %13:i32 = add %x_42, 1i
        %14:bool = lt %x_40, %13
        if %14 [t: $B5, f: $B6] {  # if_1
          $B5: {  # true
            exit_if  # if_1
          }
          $B6: {  # false
            exit_loop  # loop_1
          }
        }
        %15:i32 = load %i
        %x_46:i32 = let %15
        %17:ptr<uniform, vec4<u32>, read> = access %x_7, 0u
        %18:u32 = load_vector_element %17, 0u
        %19:i32 = bitcast %18
        %x_48:i32 = let %19
        %21:i32 = load %i
        %x_49:i32 = let %21
        %23:i32 = max %x_46, %x_48
        %24:i32 = min %23, %x_49
        %25:u32 = mul %24, 4u
        %26:u32 = div %25, 16u
        %27:ptr<uniform, vec4<u32>, read> = access %x_7, %26
        %28:u32 = mod %25, 16u
        %29:u32 = div %28, 4u
        %30:u32 = load_vector_element %27, %29
        %31:i32 = bitcast %30
        %x_52:i32 = let %31
        %33:bool = eq %x_52, 1i
        if %33 [t: $B7, f: $B8] {  # if_2
          $B7: {  # true
            %34:i32 = load %i
            %x_57:i32 = let %34
            store_vector_element %a, %x_57, 5i
            exit_if  # if_2
          }
          $B8: {  # false
            %36:i32 = load %i
            %x_59:i32 = let %36
            %38:i32 = load %i
            %x_60:i32 = let %38
            store_vector_element %a, %x_59, %x_60
            exit_if  # if_2
          }
        }
        continue  # -> $B4
      }
      $B4: {  # continuing
        %40:i32 = load %i
        %x_62:i32 = let %40
        %42:i32 = add %x_62, 1i
        store %i, %42
        next_iteration  # -> $B3
      }
    }
    %43:i32 = load_vector_element %a, 0u
    %x_65:i32 = let %43
    %45:i32 = load_vector_element %a, 1u
    %x_67:i32 = let %45
    %47:i32 = load_vector_element %a, 2u
    %x_70:i32 = let %47
    %49:i32 = load_vector_element %a, 3u
    %x_73:i32 = let %49
    %51:i32 = add %x_65, %x_67
    %52:i32 = add %51, %x_70
    %53:i32 = add %52, %x_73
    store %sum, %53
    %54:i32 = load %sum
    %x_75:i32 = let %54
    %56:bool = eq %x_75, 10i
    if %56 [t: $B9, f: $B10] {  # if_3
      $B9: {  # true
        store %x_GLF_color, vec4<f32>(1.0f, 0.0f, 0.0f, 1.0f)
        exit_if  # if_3
      }
      $B10: {  # false
        store %x_GLF_color, vec4<f32>(0.0f)
        exit_if  # if_3
      }
    }
    ret
  }
}
%main = @fragment func():main_out {
  $B11: {
    %58:void = call %main_1
    %59:vec4<f32> = load %x_GLF_color
    %60:main_out = construct %59
    ret %60
  }
}

