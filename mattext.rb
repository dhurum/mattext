require 'formula'

class Mattext < Formula
  homepage 'https://github.com/dhurum/mattext'
  url 'https://github.com/dhurum/mattext/archive/v0.7.tar.gz'
  sha1 'dc49b8cff889a9575334f303110dbb16053af00bfbd671fc75b32937be95acd7'
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
