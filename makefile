a.out: server.c response.o request.o template.o dictionary.o auth.o
	gcc -g server.c response.o request.o dictionary.o template.o auth.o -lpthread
# ./a.out: CBack.c CBack.h server.c
# 	gcc server.o CBack.c -lpthread

# server.o: server.c response.h request.h template.h dictionary.h auth.h
# 	gcc -c server.c 

auth.o : request.h auth.c template.h common.h dictionary.h response.h
	gcc -c auth.c

response.o: response.c response.h common.h
	gcc -c response.c

request.o: request.c request.h common.h
	gcc -c request.c

template.o: template.c template.h common.h request.h response.h dictionary.h auth.h
	gcc -c template.c

dictionary.o: dictionary.c dictionary.h
	gcc -c dictionary.c

clean:
	rm -f response.o request.o dictionary.o template.o auth.o a.out