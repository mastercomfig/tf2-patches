{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    gcc
    gnumake

    libunwind

    # keep this line if you use bash
    bashInteractive
  ];
}
