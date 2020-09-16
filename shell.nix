{ pkgs ? (import <nixpkgs> {}).pkgsi686Linux }:

let ogl = pkgs.linuxPackages.nvidia_x11;
in pkgs.mkShell {
  buildInputs = with pkgs; [
    gcc
    gnumake
    automake
    autoconf

    glib
    libunwind
    freetype
    fontconfig
    libGL
    xorg.libX11
    openal
    ncurses

    # keep this line if you use bash
    bashInteractive
  ];
}
