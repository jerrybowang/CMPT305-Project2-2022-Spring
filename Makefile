####################################################################
#
# Makefile for CMPT 125 Summer 2020, SFU Burnaby.
#
# Includes settings for C (gcc) and C++ (g++).
#
#####################################################################

#
# C compiler options
#

# -Wall reports all warnings 
# -Wextra turns on even more warnings
# -Werror treats warnings as if they were errors (stopping compilation)
# -Wfatal-errors causes compilation to stop after the first error
# -g adds debugging information to the file
CFLAGS=-Wall -Wextra -Werror -Wfatal-errors -g

# -lm links the math library, so that programs that include math.h will link;
#     this option can be removed for programs that don't include math.h
LDLIBS=-lm

# Set the C++ compiler options:
# -std=c++14 compiles using the C++14 standard (or at least as 
#  much as is implemented by the compiler, e.g. for g++ see
#  https://gcc.gnu.org/projects/cxx-status.html)
# -Wall turns on all warnings
# -Wextra turns on even more warnings
# -Werror causes warnings to be errors 
# -Wfatal-errors stops the compiler after the first error
# -Wno-sign-compare turns off warnings for comparing signed and 
#  unsigned numbers
# -Wnon-virtual-dtor warns about non-virtual destructors
# -g puts debugging info into the executables (makes them larger)
# -Wno-deprecated, Do not warn about usage of deprecated features
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror -Wno-deprecated -Wfatal-errors -Wno-sign-compare\
           -Wnon-virtual-dtor -g

clean:
	rm -f core *.o proj1