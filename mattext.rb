require 'formula'

class Mattext < Formula
  homepage 'https://github.com/dhurum/mattext'
  url 'https://github.com/dhurum/mattext/archive/v0.6.tar.gz'
  sha1 '8b96dd4fa1cb06bc3e0b7aade6f8e3e20910dc7b'
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
