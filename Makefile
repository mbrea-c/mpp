all : src/mpp.c
	cc src/mpp.c -o ./mpp
clean :
	rm mpp
