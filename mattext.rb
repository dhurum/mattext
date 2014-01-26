require 'formula'

class Mattext < Formula
  homepage 'https://github.com/dhurum/mattext'
  url 'https://github.com/dhurum/mattext/archive/v0.7.tar.gz'
  sha1 '79393ffabc7d3cbeb2fa02f35ea9474f3b92f55ccae8a51a80721902b27472db'
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
