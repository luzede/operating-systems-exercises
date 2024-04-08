# variable := tesla

# hello:
# 	echo "Hello, World"

# blah:
# 	echo "${variable}"

# all: hello blah

# hey: one two
# 	# Outputs "hey", since this is the target name
# 	echo $@

# 	# Outputs all prerequisites newer than the target
# 	echo $?

# 	# Outputs all prerequisites
# 	echo $^

# 	touch hey

# one:
# 	touch one

# two:
# 	touch two


CC := gcc

gates: child
	${CC} -Wall parent.c -o gates


child:
	${CC} -Wall child.c -o child

clean:
	rm -f child
	rm -f gates