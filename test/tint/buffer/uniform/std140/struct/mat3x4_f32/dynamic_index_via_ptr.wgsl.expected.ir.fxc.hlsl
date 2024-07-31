SKIP: FAILED


struct Inner {
  @size(64)
  m : mat3x4<f32>,
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
  let l_a_i_a_i_m : mat3x4<f32> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec4<f32> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f32 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :45:24 error: binary: %33 is not in scope
    %32:u32 = add %31, %33
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:56:5 note: %33 declared here
    %33:u32 = mul %44, 4u
    ^^^^^^^

:56:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %33:u32 = mul %44, 4u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(16) {
  m:mat3x4<f32> @offset(0)
}

Outer = struct @align(16) {
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
    %16:u32 = mul 16u, %15
    %17:array<Outer, 4> = call %18, 0u
    %l_a:array<Outer, 4> = let %17
    %20:Outer = call %21, %10
    %l_a_i:Outer = let %20
    %23:array<Inner, 4> = call %24, %10
    %l_a_i_a:array<Inner, 4> = let %23
    %26:u32 = add %10, %13
    %27:Inner = call %28, %26
    %l_a_i_a_i:Inner = let %27
    %30:u32 = add %10, %13
    %31:u32 = add %30, %16
    %32:u32 = add %31, %33
    %34:mat3x4<f32> = call %35, %32
    %l_a_i_a_i_m:mat3x4<f32> = let %34
    %37:u32 = add %10, %13
    %38:u32 = add %37, %16
    %39:u32 = div %38, 16u
    %40:ptr<uniform, vec4<u32>, read> = access %a, %39
    %41:vec4<u32> = load %40
    %42:vec4<f32> = bitcast %41
    %l_a_i_a_i_m_i:vec4<f32> = let %42
    %44:i32 = call %i
    %33:u32 = mul %44, 4u
    %45:u32 = add %10, %13
    %46:u32 = add %45, %16
    %47:u32 = add %46, %33
    %48:u32 = div %47, 16u
    %49:ptr<uniform, vec4<u32>, read> = access %a, %48
    %50:u32 = mod %47, 16u
    %51:u32 = div %50, 4u
    %52:u32 = load_vector_element %49, %51
    %53:f32 = bitcast %52
    %l_a_i_a_i_m_i_i:f32 = let %53
    ret
  }
}
%21 = func(%start_byte_offset:u32):Outer {
  $B4: {
    %56:array<Inner, 4> = call %24, %start_byte_offset
    %57:Outer = construct %56
    ret %57
  }
}
%24 = func(%start_byte_offset_1:u32):array<Inner, 4> {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat3x4<f32>(vec4<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B6, b: $B7, c: $B8] {  # loop_1
      $B6: {  # initializer
        next_iteration 0u  # -> $B7
      }
      $B7 (%idx:u32): {  # body
        %61:bool = gte %idx, 4u
        if %61 [t: $B9] {  # if_1
          $B9: {  # true
            exit_loop  # loop_1
          }
        }
        %62:u32 = mul %idx, 64u
        %63:u32 = add %start_byte_offset_1, %62
        %64:ptr<function, Inner, read_write> = access %a_1, %idx
        %65:Inner = call %28, %63
        store %64, %65
        continue  # -> $B8
      }
      $B8: {  # continuing
        %66:u32 = add %idx, 1u
        next_iteration %66  # -> $B7
      }
    }
    %67:array<Inner, 4> = load %a_1
    ret %67
  }
}
%28 = func(%start_byte_offset_2:u32):Inner {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %69:mat3x4<f32> = call %35, %start_byte_offset_2
    %70:Inner = construct %69
    ret %70
  }
}
%35 = func(%start_byte_offset_3:u32):mat3x4<f32> {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %72:u32 = div %start_byte_offset_3, 16u
    %73:ptr<uniform, vec4<u32>, read> = access %a, %72
    %74:vec4<u32> = load %73
    %75:vec4<f32> = bitcast %74
    %76:u32 = add 16u, %start_byte_offset_3
    %77:u32 = div %76, 16u
    %78:ptr<uniform, vec4<u32>, read> = access %a, %77
    %79:vec4<u32> = load %78
    %80:vec4<f32> = bitcast %79
    %81:u32 = add 32u, %start_byte_offset_3
    %82:u32 = div %81, 16u
    %83:ptr<uniform, vec4<u32>, read> = access %a, %82
    %84:vec4<u32> = load %83
    %85:vec4<f32> = bitcast %84
    %86:mat3x4<f32> = construct %75, %80, %85
    ret %86
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat3x4<f32>(vec4<f32>(0.0f))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %90:bool = gte %idx_1, 4u
        if %90 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %91:u32 = mul %idx_1, 256u
        %92:u32 = add %start_byte_offset_4, %91
        %93:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %94:Outer = call %21, %92
        store %93, %94
        continue  # -> $B15
      }
      $B15: {  # continuing
        %95:u32 = add %idx_1, 1u
        next_iteration %95  # -> $B14
      }
    }
    %96:array<Outer, 4> = load %a_2
    ret %96
  }
}

