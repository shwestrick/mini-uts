{ pkgs   ? import <nixpkgs> {},
  stdenv ? pkgs.stdenv,
  miniUTSSrc ? ../.
}:

stdenv.mkDerivation rec {
  name = "mini-UTS";

  src = miniUTSSrc;

  installPhase = ''
    mkdir -p $out/rng/
    cp rng/* $out/rng/
    cp *.c *.cpp *.h $out/  
  '';

  meta = {
    description = "Mini UTS.";
    license = "MIT";
  };
}
