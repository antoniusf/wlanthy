{ pkgs ? import <nixpkgs> {} }:

let anthy-unicode = pkgs.callPackage /home/thekla/src/nixpkgs/pkgs/tools/inputmethods/anthy/default.nix {}; in
pkgs.callPackage ./derivation.nix { inherit anthy-unicode; }
