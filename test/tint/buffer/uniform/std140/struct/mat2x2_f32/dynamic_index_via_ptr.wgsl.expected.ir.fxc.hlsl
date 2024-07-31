SKIP: FAILED


struct Inner {
  @size(64)
  m : mat2x2<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

var<private> counter = 0;

fn i() -> i32 {
  counter++;
  return counter;
}

@compute @workgroup_size(1)
fn f() {
  let p_a = &(a);
  let p_a_i = &((*(p_a))[i()]);
  let p_a_i_a = &((*(p_a_i)).a);
  let p_a_i_a_i = &((*(p_a_i_a))[i()]);
  let p_a_i_a_i_m = &((*(p_a_i_a_i)).m);
  let p_a_i_a_i_m_i = &((*(p_a_i_a_i_m))[i()]);
  let l_a : array<Outer, 4> = *(p_a);
  let l_a_i : Outer = *(p_a_i);
  let l_a_i_a : array<Inner, 4> = *(p_a_i_a);
  let l_a_i_a_i : Inner = *(p_a_i_a_i);
  let l_a_i_a_i_m : mat2x2<f32> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec2<f32> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f32 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :38:24 error: binary: %23 is not in scope
    %22:u32 = add %21, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:65:5 note: %23 declared here
    %23:u32 = mul %53, 4u
    ^^^^^^^

:48:24 error: binary: %23 is not in scope
    %36:u32 = add %35, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:65:5 note: %23 declared here
    %23:u32 = mul %53, 4u
    ^^^^^^^

:65:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %23:u32 = mul %53, 4u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(8) {
  m:mat2x2<f32> @offset(0)
}

Outer = struct @align(8) {
  a:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %a:ptr<uniform, array<vec4<u32>, 64>, read> = var @binding_point(0, 0)
  %counter:ptr<private, i32, read_write> = var, 0i
}

%i = func():i32 {
  $B2: {
    %4:i32 = load %counter
    %5:i32 = add %4, 1i
    store %counter, %5
    %6:i32 = load %counter
    ret %6
  }
}
%f = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:i32 = call %i
    %9:u32 = convert %8
    %10:u32 = mul 256u, %9
    %11:i32 = call %i
    %12:u32 = convert %11
    %13:u32 = mul 64u, %12
    %14:i32 = call %i
    %15:u32 = convert %14
    %16:u32 = mul 8u, %15
    %17:array<Outer, 4> = call %18, 0u
    %l_a:array<Outer, 4> = let %17
    %20:u32 = add %10, %13
    %21:u32 = add %20, %16
    %22:u32 = add %21, %23
    %24:Outer = call %25, %22
    %l_a_i:Outer = let %24
    %27:array<Inner, 4> = call %28, %10
    %l_a_i_a:array<Inner, 4> = let %27
    %30:u32 = add %10, %13
    %31:Inner = call %32, %30
    %l_a_i_a_i:Inner = let %31
    %34:u32 = add %10, %13
    %35:u32 = add %34, %16
    %36:u32 = add %35, %23
    %37:mat2x2<f32> = call %38, %36
    %l_a_i_a_i_m:mat2x2<f32> = let %37
    %40:u32 = add %10, %13
    %41:u32 = add %40, %16
    %42:u32 = div %41, 16u
    %43:ptr<uniform, vec4<u32>, read> = access %a, %42
    %44:u32 = mod %41, 16u
    %45:u32 = div %44, 4u
    %46:vec4<u32> = load %43
    %47:vec2<u32> = swizzle %46, zw
    %48:vec2<u32> = swizzle %46, xy
    %49:bool = eq %45, 2u
    %50:vec2<u32> = hlsl.ternary %48, %47, %49
    %51:vec2<f32> = bitcast %50
    %l_a_i_a_i_m_i:vec2<f32> = let %51
    %53:i32 = call %i
    %23:u32 = mul %53, 4u
    %54:u32 = add %10, %13
    %55:u32 = add %54, %16
    %56:u32 = add %55, %23
    %57:u32 = div %56, 16u
    %58:ptr<uniform, vec4<u32>, read> = access %a, %57
    %59:u32 = mod %56, 16u
    %60:u32 = div %59, 4u
    %61:u32 = load_vector_element %58, %60
    %62:f32 = bitcast %61
    %l_a_i_a_i_m_i_i:f32 = let %62
    ret
  }
}
%28 = func(%start_byte_offset:u32):array<Inner, 4> {
  $B4: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat2x2<f32>(vec2<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B5, b: $B6, c: $B7] {  # loop_1
      $B5: {  # initializer
        next_iteration 0u  # -> $B6
      }
      $B6 (%idx:u32): {  # body
        %67:bool = gte %idx, 4u
        if %67 [t: $B8] {  # if_1
          $B8: {  # true
            exit_loop  # loop_1
          }
        }
        %68:u32 = mul %idx, 64u
        %69:u32 = add %start_byte_offset, %68
        %70:ptr<function, Inner, read_write> = access %a_1, %idx
        %71:Inner = call %32, %69
        store %70, %71
        continue  # -> $B7
      }
      $B7: {  # continuing
        %72:u32 = add %idx, 1u
        next_iteration %72  # -> $B6
      }
    }
    %73:array<Inner, 4> = load %a_1
    ret %73
  }
}
%32 = func(%start_byte_offset_1:u32):Inner {  # %start_byte_offset_1: 'start_byte_offset'
  $B9: {
    %75:mat2x2<f32> = call %38, %start_byte_offset_1
    %76:Inner = construct %75
    ret %76
  }
}
%38 = func(%start_byte_offset_2:u32):mat2x2<f32> {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %78:u32 = div %start_byte_offset_2, 16u
    %79:ptr<uniform, vec4<u32>, read> = access %a, %78
    %80:u32 = mod %start_byte_offset_2, 16u
    %81:u32 = div %80, 4u
    %82:vec4<u32> = load %79
    %83:vec2<u32> = swizzle %82, zw
    %84:vec2<u32> = swizzle %82, xy
    %85:bool = eq %81, 2u
    %86:vec2<u32> = hlsl.ternary %84, %83, %85
    %87:vec2<f32> = bitcast %86
    %88:u32 = add 8u, %start_byte_offset_2
    %89:u32 = div %88, 16u
    %90:ptr<uniform, vec4<u32>, read> = access %a, %89
    %91:u32 = mod %88, 16u
    %92:u32 = div %91, 4u
    %93:vec4<u32> = load %90
    %94:vec2<u32> = swizzle %93, zw
    %95:vec2<u32> = swizzle %93, xy
    %96:bool = eq %92, 2u
    %97:vec2<u32> = hlsl.ternary %95, %94, %96
    %98:vec2<f32> = bitcast %97
    %99:mat2x2<f32> = construct %87, %98
    ret %99
  }
}
%25 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %101:array<Inner, 4> = call %28, %start_byte_offset_3
    %102:Outer = construct %101
    ret %102
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat2x2<f32>(vec2<f32>(0.0f))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %106:bool = gte %idx_1, 4u
        if %106 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %107:u32 = mul %idx_1, 256u
        %108:u32 = add %start_byte_offset_4, %107
        %109:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %110:Outer = call %25, %108
        store %109, %110
        continue  # -> $B15
      }
      $B15: {  # continuing
        %111:u32 = add %idx_1, 1u
        next_iteration %111  # -> $B14
      }
    }
    %112:array<Outer, 4> = load %a_2
    ret %112
  }
}

