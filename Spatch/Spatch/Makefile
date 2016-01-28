DIR= 	./SpatchServer/
SRC= 	$(DIR)/main.cpp\
	$(DIR)/SPTServ.cpp\
	$(DIR)/SPTServGuest.cpp\
    $(DIR)/SPTServGuestTalk.cpp\

make:
	clang++ -o spatch $(SRC) -L/usr/lib -lssh -lutil
