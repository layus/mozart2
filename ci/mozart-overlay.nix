self: super:

with self;

{ 
  mozart2 = llvmPackages.stdenv.mkDerivation rec {
    name = "mozart2-${version}";
    version = "2.0.0-beta.1";

    src = if super.lib.inNixShell then null 
      else super.fetchFromGitHub {
        repo = "mozart2";
        owner = "mozart";
        rev = "78c9e389d5127a3c80bce8be69c21be578b6af17";
        sha256 = "1vwpphdq5jk60wvvvc39xah2bm9v884ykypa6dyjjrgl73izd5ay";
        fetchSubmodules = true;
      };

    bootcompiler = fetchurl {
      url = "https://github.com/layus/mozart2/releases/download/v2.0.0-beta.1/bootcompiler.jar";
      sha256 = "1hgh1a8hgzgr6781as4c4rc52m2wbazdlw3646s57c719g5xphjz";
    };

    postConfigure = ''
      ln -sfn ${bootcompiler} bootcompiler/bootcompiler.jar
    '';

    nativeBuildInputs = [ cmake makeWrapper ];

    preConfigure = ''
      cmakeFlagsArray+=( -DCMAKE_CXX_FLAGS="-Wno-braced-scalar-init -fsanitize=thread -g" )
    '';
    cmakeFlags = [ 
      "-DCMAKE_CXX_COMPILER=${llvmPackages.clang}/bin/clang++"
      "-DCMAKE_C_COMPILER=${llvmPackages.clang}/bin/clang"
      "-DBoost_USE_STATIC_LIBS=OFF"
      "-DMOZART_BOOST_USE_STATIC_LIBS=OFF"
      "-DCMAKE_PROGRAM_PATH=${llvmPackages_4.clang}/bin"
      # Rationale: Nix's cc-wrapper needs to see a compile flag (like -c) to
      # infer that it is not a linking call, and stop trashing the command line
      # with linker flags.
      # As it does not recognise -emit-ast, we pass -c immediately overridden
      # by -emit-ast.
      # The remaining is just the default flags that we cannot reuse and need
      # to repeat here.
      "-DMOZART_GENERATOR_FLAGS='-c;-emit-ast;--std=c++0x;-Wno-invalid-noreturn;-Wno-return-type;-Wno-braced-scalar-init'"
      # We are building with clang, as nix does not support having clang and
      # gcc together as compilers and we need clang for the sources generation.
      # However, clang emits tons of warnings about gcc's atomic-base library.
      #"-DCMAKE_CXX_FLAGS=-Wno-braced-scalar-init"
    ];

    fixupPhase = ''
      wrapProgram $out/bin/oz --set OZEMACS ${emacs}/bin/emacs
    '';

    doCheck = true;

    buildInputs = [
      boost
      llvmPackages_4.llvm
      llvmPackages_4.clang
      llvmPackages_4.clang-unwrapped
      gmp
      emacs25-nox
      jre_headless
      tcl
      tk
    ];

  };
}

