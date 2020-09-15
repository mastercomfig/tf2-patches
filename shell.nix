{ pkgs ? (import <nixpkgs> {}).pkgsi686Linux }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    gcc
    gnumake
    automake
    autoconf

    glib
    libunwind

    # keep this line if you use bash
    bashInteractive
  ];
}
