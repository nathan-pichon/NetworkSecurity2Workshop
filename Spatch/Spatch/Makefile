DIR= 	./SpatchServer/
SRC= 	$(DIR)/main.cpp\
        $(DIR)/SPTMain.cpp\
        $(DIR)/SPTBinder.cpp\
        $(DIR)/SPTGuestController.cpp\
        $(DIR)/SPTRemoteController.cpp\
        $(DIR)/SPTTools.cpp\

make:
	clang++ -o spatch $(SRC) -L/usr/lib -lssh -lutil -std=c++11
