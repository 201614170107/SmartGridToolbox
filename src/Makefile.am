AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS += -I../SgtSim

bin_PROGRAMS = run_config solve_network solve_matpower_network

run_config_SOURCES = run_config.cc

run_config_LDADD = ../SgtSim/libSgtSim.la ../SgtCore/libSgtCore.la 

solve_network_SOURCES = solve_network.cc

solve_network_LDADD = ../SgtCore/libSgtCore.la

solve_matpower_network_SOURCES = solve_matpower_network.cc

solve_matpower_network_LDADD = ../SgtCore/libSgtCore.la
