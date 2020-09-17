{ pkgs ? (import <nixpkgs> {}).pkgsi686Linux,
 x64pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    gcc
    gnumake
    automake
    autoconf
    glxinfo

    x64pkgs.lldb

    SDL2
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

  LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath (with pkgs; [
    SDL2
    xorg.libX11
    xorg.libXext
    xorg.libXinerama
    xorg.libXi
    xorg.libXrandr
    libGL
  ]);
}
