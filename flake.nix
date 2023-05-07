{
  description = "WebGPU for C++";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/master";

    utils.url = "github:numtide/flake-utils";
    utils.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs = { self, nixpkgs, ... }@inputs:
    inputs.utils.lib.eachSystem [
      "x86_64-linux"
      "i686-linux"
      "aarch64-linux"
      "x86_64-darwin"
    ] (system:
      let
        pkgs = import nixpkgs { inherit system; };
        llvm = pkgs.llvmPackages_11;
        lib = nixpkgs.lib;
      in {
        devShell = pkgs.mkShell rec {
          name = "webgpu-cpp";

          packages = with pkgs; [ ];

          nativeBuildInputs = with pkgs; [
            # Development Tools
            llvm.lldb
            cmake
            cmakeCurses
            boost
            glfw-wayland
            wayland
            python3

            # X11 dependencies
            xorg.libX11
            xorg.libXrandr
            xorg.libXinerama
            xorg.libXcursor
            xorg.libXi

            ecm
            libGL
            libxkbcommon
            wayland-protocols
            extra-cmake-modules
            libffi

            clang-tools
            llvm.clang
          ];

          cmakeFlags = [ "-DGLFW_USE_WAYLAND=ON" ];

          buildInputs = with pkgs;
            [
              # stdlib for cpp
              llvm.libcxx
            ];

          CXXFLAGS = "-std=c++20";
          # CPATH = builtins.concatStringsSep ":" [
          # (lib.makeSearchPathOutput "dev" "include" [ llvm.libcxx ])
          # (lib.makeSearchPath "resource-root/include" [ llvm.clang ])
          # ];

          # Setting up the environment variables you need during
          # development.
          shellHook = let icon = "f121";
          in ''
            export PS1="$(echo -e '\u${icon}') {\[$(tput sgr0)\]\[\033[38;5;228m\]\w\[$(tput sgr0)\]\[\033[38;5;15m\]} (${name}) \\$ \[$(tput sgr0)\]"
            export SHELL="${pkgs.zsh}/bin/zsh"
          '';
        };
      });
}
