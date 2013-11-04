CPPFLAGS := -Werror \
            -Wall \
            -Wuninitialized \
            -Winit-self \
            -Wtype-limits \
            -Wclobbered \
            -Wempty-body 

mattext: mattext.cpp
	g++ mattext.cpp -o mattext -lcurses $(CPPFLAGS)

clean:
	rm -f mattext
