{
  description = "A flake for c";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    chimera.url = "github:secretval/chimera";
  };

  outputs = inputs @ {flake-parts, ...}:
    flake-parts.lib.mkFlake {inherit inputs;} {
      systems = ["x86_64-linux" "aarch64-linux" "aarch64-darwin" "x86_64-darwin"];

      perSystem = {
        self',
        pkgs,
        ...
      }:  {
        devShells.default = pkgs.mkShell {
            inputsFrom = [
              self'.packages.vic
              self'.packages.vism
            ];
            buildInputs = with pkgs; [uncrustify];
        };

          packages = {
            vic  = pkgs.callPackage ./lib/buildPackage.nix {name = "vic"; version  = "0.0.1";chimera = inputs.chimera;};
            vism = pkgs.callPackage ./lib/buildPackage.nix {name = "vism"; version = "0.0.1";chimera = inputs.chimera;};
        };
      };
    };
}
