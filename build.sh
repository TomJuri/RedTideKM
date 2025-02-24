#/bin/bash
mkdir build
make -C $(nix-build -E '(import <nixpkgs> {}).linux.dev' --no-out-link)/lib/modules/*/build M=$(pwd) O=$(pwd)/build modules
