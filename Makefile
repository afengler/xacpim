CFLAGS	+= -L${LOCALBASE}/lib -I${LOCALBASE}/include

OBJS	= main.o graphics.o sys.o osdep/freebsd.o

all::	${OBJS}
	${CC} -o xacpim ${OBJS} -lX11 -L${LOCALBASE}/lib -I${LOCALBASE}/include 

install:: xacpim 
	strip xacpim 
	cp xacpim /usr/X11R6/bin
	chmod 0755 /usr/X11R6/bin/xacpim

clean::
	rm *.o
	rm xacpim
