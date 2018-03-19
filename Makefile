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
SCALE 	:= -s  0.00000004
ITERS 	:= -m 1000
WIDTH 	:= -W 1024
HEIGHT 	:= -H 1024
OUTPUT 	:= -o $(OUT)/mandel.bmp
THREADS := -n 4
CHILDREN:= 4
PARAMS 	:=  $(X) $(Y) $(SCALE) $(ITERS) $(WIDTH) $(HEIGHT) $(OUTPUT)

all: mkdirs $(PRODUCT)

workstealing: PARAMS += -w
.PHONY: workstealing

timing: CFLAGS += -DTIMING
timing: mkdirs $(PRODUCT)
.PHONY: timing

tests: mkdirs $(TESTS)
	./$(BIN)/mandel $(PARAMS) $(THREADS)
	./$(BIN)/mandelseries $(PARAMS) $(CHILDREN)
.PHONY: tests

tmandel: timing $(TESTS)
	@./$(BIN)/mandel $(PARAMS) -n  1 > /dev/null
	@./$(BIN)/mandel $(PARAMS) -n  2 > /dev/null
	@./$(BIN)/mandel $(PARAMS) -n  3 > /dev/null
	@./$(BIN)/mandel $(PARAMS) -n  4 > /dev/null
	@./$(BIN)/mandel $(PARAMS) -n  5 > /dev/null
	@./$(BIN)/mandel $(PARAMS) -n 10 > /dev/null
	@./$(BIN)/mandel $(PARAMS) -n 50 > /dev/null
.PHONY: tmandel

tseries: timing $(TESTS)
	./$(BIN)/mandelseries $(PARAMS) $(CHILDREN) > /dev/null
.PHONY: tseries

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

