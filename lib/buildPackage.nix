{
pkgs,
name,
version,
chimera,
...
}:
pkgs.stdenv.mkDerivation {
  inherit version;
  pname = name;
  src = ./../.;

  buildInputs = with pkgs; [gcc chimera.packages.${system}.default];

  buildPhase = ''
    make -B
    ./make
    '';

  installPhase = ''
    mkdir -p $out/bin
    mv build/${name} $out/bin
    '';
}
