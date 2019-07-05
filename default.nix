{ pkgs ? import <nixpkgs> {}
, lib ? pkgs.lib
}:

pkgs.stdenv.mkDerivation {

  pname = "fart";
  version = "git";

  src = lib.cleanSource ./.;

  buildPhase = ''
    patchShebangs ./mk
    ./mk
  '';

  installPhase = ''
    mkdir -p $out/bin
    install -Dm755 fart $out/bin/fart
  '';

}
