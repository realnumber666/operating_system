GCC = gcc
EXEC = multisum
SRC = $(shell find ./ -name "*.c")
OBJ = $(SRC:%.c=%.o)       

$(EXEC): $(OBJ)
	$(GCC) -o $(EXEC) $(OBJ) -lpthread

%.o: %.c
	$(GCC) -c $< -o $@ -lpthread    
	
test:
	$(shell sh ./test.sh) 

clean:
	rm -rf $(OBJ) $(EXEC)