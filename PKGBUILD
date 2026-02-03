# Maintainer: Andromeda <support@andromeda.os>
pkgname=aero-control-panel
pkgver=1.0.0
pkgrel=1
pkgdesc="A modern Qt-based Control Panel with Aero aesthetics"
arch=('x86_64')
url="https://github.com/andromeda/aero_panel_cpp"
license=('MIT')
depends=('qt6-base')
makedepends=('cmake' 'ninja' 'gcc')
source=("$pkgname-$pkgver.tar.gz") # Placeholder, usually points to git tag
md5sums=('SKIP')

build() {
    cd "$srcdir/../"  # Assuming we run this in place or copy the whole dir
    cmake -B build -S . \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build
}

package() {
    cd "$srcdir/../"
    DESTDIR="$pkgdir" cmake --install build
}
