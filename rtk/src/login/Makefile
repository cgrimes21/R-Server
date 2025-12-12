
%.o: %.c
	@echo -e " Compile... $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

all: login-server

login-server: login.o login_client.o intif.o $(COMMON_OBJ)
	@echo -e " Linking... $<"
	@$(CC) -o ../../$@ login.o login_client.o intif.o $(COMMON_OBJ) $(CLIBS)

login.o: login.c login.h login_client.h zlib.h $(COMMON_H)
login_client.o: login_client.c login.h login_client.h intif.h $(COMMON_H)
intif.o: intif.c intif.h $(COMMON_H)
clean:
	@rm -f *.o ../../login-server
