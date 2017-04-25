AM_CPPFLAGS = $(CXX11_FLAGS) -I$(srcdir)/../src
AM_CXXFLAGS = $(PICKY_CXXFLAGS)
LDADD = ../src/libsourdough.a -lpthread

common_source = contest_message.hh contest_message.cc \
	controller.hh

default_controller = default_controller.hh default_controller.cc
exercise_a_source = exercisea.hh exercisea.cc
exercise_b_source = exerciseb.hh exerciseb.cc
exercise_c_source = exercisec.hh exercisec.cc
exercise_d_source = exercised.hh exercised.cc
exercise_sources = $(default_controller) $(exercise_a_source) $(exercise_b_source) $(exercise_c_source) $(exercise_d_source)

bin_PROGRAMS = sender receiver

sender_SOURCES = $(common_source) $(exercise_sources) sender.cc

receiver_SOURCES = $(common_source) receiver.cc
