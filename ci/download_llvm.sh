#!/usr/bin/env bash

[ $# -eq 2 ] || exit
download_root=$(realpath -m "$1")
install_root=$(realpath -m "$2")

v=4.0
version=$v~svn305264-1~exp1_amd64.deb

get_llvm () {
    local file=$1_$version

    apt_url=https://apt.llvm.org/trusty/pool/main/l/llvm-toolchain-$v
    # --no-clobber. We assume identical names have identical content.
    wget "$apt_url/$file" --no-clobber

    ar p "$file" data.tar.xz | (cd "$install_root" || exit; tar xJ)
}

deps="clang-$v clang-format-$v clang-tidy-$v libclang-$v-dev libclang1-$v"
deps+=" libclang-common-$v-dev llvm-$v-dev llvm-$v-runtime llvm-$v libllvm$v"

rm -rf "$install_root"
mkdir -p "$install_root" "$download_root"
cd "$download_root" || exit
for deb in $deps; do
    get_llvm "$deb"
done

relink () {
    file=$1; shift
    for target in "$@"
    do
        ln -sfn "$target" "$file"
    done
}

grep -l /usr/lib/llvm-4.0 -r "$install_root" | xargs sed -i "s#/usr/lib/llvm-4.0#$install_root&#g"

p=$install_root
ln -sfn "$p"/usr/share/llvm-4.0/cmake "$p"/usr/lib/llvm-4.0/lib/cmake/clang
ln -sfn "$p"/usr/share/llvm-4.0/cmake "$p"/usr/lib/llvm-4.0/lib/cmake/clang-4.0
ln -sfn "$p"/usr/bin/*                "$p"/usr/lib/llvm-4.0/bin || true



