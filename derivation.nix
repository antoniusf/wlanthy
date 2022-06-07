{ lib, stdenv, cmake, meson, ninja, pkg-config, anthy, wayland, wayland-protocols, libxkbcommon, anthy-unicode }:

stdenv.mkDerivation {
  name = "wlanthy-oyayubi";
  #src = builtins.fetchGit {
  #  url = "git@github.com:antoniusf/wlanthy.git";
  #  ref = "thumb-shift";
  #  rev = "c7d749a2725092be8350cf23433d47d95c03a548";
  #};
  src = ./.;

  buildInputs = [ meson ninja cmake pkg-config anthy-unicode wayland wayland-protocols libxkbcommon ];
}
