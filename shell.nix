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
    (curl.override { gnutlsSupport = true; sslSupport = false; })

    # keep this line if you use bash
    bashInteractive
  ];
}
