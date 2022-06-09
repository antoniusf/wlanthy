{ lib, stdenv, fetchurl, autoreconfHook }:

stdenv.mkDerivation rec {
  pname = "anthy";
  version = "1.0.0.20211224";
  
  buildInputs = [ autoreconfHook ];

  src = fetchGit {
    url = "https://github.com/fujiwarat/anthy-unicode.git";
    ref = "refs/tags/${version}";
  };
}
