{ pkgs ? import <nixpkgs> {} }:

let anthy-unicode = pkgs.callPackage ./anthy-unicode.nix {}; in
pkgs.callPackage ./derivation.nix { inherit anthy-unicode; }
