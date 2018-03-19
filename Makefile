PRODUCT := mandel mandelseries

BIN 	:= bin
OBJ 	:= obj
OUT 	:= out
SRC 	:= src
INC 	:= include

CC	:= gcc
INCDIRS := -I$(INC)
CFLAGS	:= -Wall -Wextra -Werror -g
LIBS	:= -pthread -lm

SRCS 	:= $(wildcard $(SRC)/*.c)
MAINS 	:= $(patsubst %, $(SRC)/%.c, $(PRODUCT))
SRCS 	:= $(filter-out $(MAINS), $(SRCS))
OBJS 	:= $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

# Image Parameters for testing
X 	:= -x -0.235125
Y 	:= -y  0.827215
SCALE 	:= -s  0.000004
ITERS 	:= -m 1000
WIDTH 	:= -W 1024
HEIGHT 	:= -H 1024
OUTPUT 	:= -o $(OUT)/mandel.bmp
THREADS := 4
CHILDREN:= 4
PARAMS 	:=  $(X) $(Y) $(SCALE) $(ITERS) $(WIDTH) $(HEIGHT) $(OUTPUT)

all: mkdirs $(PRODUCT)

test: all $(TESTS)
	./$(BIN)/mandel $(PARAMS) -n $(THREADS)
	./$(BIN)/mandelseries $(PARAMS) $(CHILDREN)
.PHONY: test

################################################################################
# EXECUTABLES                                                                  #
################################################################################

$(PRODUCT): $(OBJS)
	$(CC) $(CFLAGS) $(INCDIRS) $^ $(SRC)/$@.c -o $(BIN)/$@ $(LIBS)

################################################################################
# SHARED OBJECTS                                                               #
################################################################################

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) $(INCDIRS) -c $< -o $@

################################################################################
# PHONEY RULES                                                                 #
################################################################################

mkdirs:
	@mkdir -p obj
	@mkdir -p bin
.PHONY: mkdirs

clean:
	@rm -rf $(OBJ)
	@rm -rf $(BIN)
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
.PHONY: clean

