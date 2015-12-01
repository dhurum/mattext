require 'formula'

class Mattext < Formula
  homepage 'https://github.com/dhurum/mattext'
  url 'https://github.com/dhurum/mattext/archive/v0.8.tar.gz'
  sha1 '4998efdf224f8f8c7afe54e3dffe60f547bcc573b50b6d3415d52865ee519ab2'
  depends_on 'cmake' => :build
  depends_on 'argp-standalone'
  depends_on 'libev'

  def install
    system "cmake", "./", *std_cmake_args
    system "make install"
  end

  test do
    system "mattext /usr/local/Library/Formula/mattext.rb"
  end
end
