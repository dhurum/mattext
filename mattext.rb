require 'formula'

class Mattext < Formula
  homepage 'https://github.com/dhurum/mattext'
  url 'https://github.com/dhurum/mattext/archive/v0.7.tar.gz'
  sha1 '9a2a60245a79a161473b50fc86b3e0c94e79092a967869707d459531ed12fc65'
  depends_on 'cmake' => :build
  depends_on 'argp-standalone'

  def install
    system "cmake", "./", *std_cmake_args
    system "make install"
  end

  test do
    system "mattext /usr/local/Library/Formula/mattext.rb"
  end
end
