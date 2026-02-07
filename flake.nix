{
  description = "Flake for vl";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    chimera.url = "github:jt0w/chimera";
  };

  outputs = inputs @ {flake-parts, ...}:
    flake-parts.lib.mkFlake {inherit inputs;} {
      systems = ["x86_64-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin"];

      perSystem = {
        self',
        pkgs,
        ...
      }: {
        devShells.default = pkgs.mkShell {
          inputsFrom = [
            self'.packages.vl
            self'.packages.vasm
          ];
          buildInputs = with pkgs; [uncrustify universal-ctags];
          shellHook = ''
            export PATH=$PATH:$PWD/build/
          '';
        };

        packages = {
          vl = pkgs.callPackage ./lib/buildPackage.nix {
            name = "vl";
            version = "0.0.1";
            chimera = inputs.chimera;
          };
          vasm = pkgs.callPackage ./lib/buildPackage.nix {
            name = "vasm";
            version = "0.0.1";
            chimera = inputs.chimera;
          };
        };
      };
    };
}
