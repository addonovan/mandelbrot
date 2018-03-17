PRODUCT := mandel mandelseries

BIN 	:= bin
OBJ 	:= obj
SRC 	:= src
INC 	:= include

CC	:= gcc
INCDIRS := -I$(INC)
CFLAGS	:= -Wall -Wextra -Werror -lpthread -g

SRCS 	:= $(wildcard $(SRC)/*.c)
MAINS 	:= $(patsubst %, $(SRC)/%.c, $(PRODUCT))
SRCS 	:= $(filter-out $(MAINS), $(SRCS))
OBJS 	:= $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

all: mkdirs $(PRODUCT)

################################################################################
# EXECUTABLES                                                                  #
################################################################################

$(PRODUCT): $(OBJS)
	$(CC) $(CFLAGS) $(INCDIRS) $^ $(SRC)/$@.c -o $(BIN)/$@

################################################################################
# SHARED OBJECTS                                                               #
################################################################################

$(OBJ)/%.o: $(SRCS)
	$(CC) $(CFLAGS) $(INCDIRS) -c $< -o $@

################################################################################
# PHONEY RULES                                                                 #
################################################################################

mkdirs:
	@mkdir -p obj
	@mkdir -p bin
.PHONY: mkdirs

clean:
	rm obj/*
	rm bin/*
.PHONY: clean

